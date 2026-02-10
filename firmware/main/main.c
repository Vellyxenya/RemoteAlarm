#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "driver/i2s_std.h"

static const char *TAG = "REMOTE_ALARM";

// Configuration from Kconfig
#define WIFI_SSID      CONFIG_EXAMPLE_WIFI_SSID
#define WIFI_PASS      CONFIG_EXAMPLE_WIFI_PASSWORD
#define MQTT_BROKER    CONFIG_MQTT_BROKER_URL
#define MQTT_USER      CONFIG_MQTT_USERNAME
#define MQTT_PASS      CONFIG_MQTT_PASSWORD
#define MQTT_TOPIC     CONFIG_MQTT_TOPIC

// I2S configuration - adjust these pins for your board
#define I2S_BCK_IO     (GPIO_NUM_17)
#define I2S_WS_IO      (GPIO_NUM_18)
#define I2S_DO_IO      (GPIO_NUM_16)
#define I2S_SAMPLE_RATE 16000

static EventGroupHandle_t wifi_event_group;
static i2s_chan_handle_t tx_handle = NULL;

#define WIFI_CONNECTED_BIT BIT0

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying WiFi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init(void) {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi connected");
}

static void i2s_init(void) {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_IO,
            .ws = I2S_WS_IO,
            .dout = I2S_DO_IO,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_handle, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));
    ESP_LOGI(TAG, "I2S initialized");
}

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    return ESP_OK;
}

static void play_audio(const char *url) {
    ESP_LOGI(TAG, "Downloading audio from: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .buffer_size = 4096,
        .timeout_ms = 10000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    int content_length = esp_http_client_fetch_headers(client);
    ESP_LOGI(TAG, "Content length: %d", content_length);

    // Skip WAV header (44 bytes)
    char header[44];
    int read_len = esp_http_client_read(client, header, 44);
    if (read_len != 44) {
        ESP_LOGE(TAG, "Failed to read WAV header");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return;
    }

    // Stream audio data to I2S
    char *buffer = malloc(512);
    size_t bytes_written;
    int total_read = 44;

    while (1) {
        read_len = esp_http_client_read(client, buffer, 512);
        if (read_len <= 0) break;
        
        i2s_channel_write(tx_handle, buffer, read_len, &bytes_written, portMAX_DELAY);
        total_read += read_len;
    }

    free(buffer);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    ESP_LOGI(TAG, "Playback complete. Total bytes: %d", total_read);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            esp_mqtt_client_subscribe(event->client, MQTT_TOPIC, 0);
            ESP_LOGI(TAG, "Subscribed to: %s", MQTT_TOPIC);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Data received");
            cJSON *root = cJSON_ParseWithLength(event->data, event->data_len);
            if (root) {
                cJSON *url_item = cJSON_GetObjectItem(root, "file_url");
                if (cJSON_IsString(url_item) && url_item->valuestring) {
                    play_audio(url_item->valuestring);
                }
                cJSON_Delete(root);
            }
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT Error");
            break;
            
        default:
            break;
    }
}

static void mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER,
        .credentials.username = MQTT_USER,
        .credentials.authentication.password = MQTT_PASS,
    };
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "MQTT client started");
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    
    ESP_LOGI(TAG, "Starting RemoteAlarm...");
    
    wifi_init();
    i2s_init();
    mqtt_init();
    
    ESP_LOGI(TAG, "RemoteAlarm ready!");
}
