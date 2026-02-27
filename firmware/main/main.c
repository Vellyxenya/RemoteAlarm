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
#include "esp_crt_bundle.h"
#include "mqtt_client.h"
#include "esp_http_client.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "driver/i2s_std.h"
#include "esp_heap_caps.h"
#include "freertos/ringbuf.h"

static const char *TAG = "REMOTE_ALARM";

// Configuration from Kconfig
#define WIFI_SSID      CONFIG_EXAMPLE_WIFI_SSID
#define WIFI_PASS      CONFIG_EXAMPLE_WIFI_PASSWORD
#define MQTT_BROKER    CONFIG_MQTT_BROKER_URL
#define MQTT_USER      CONFIG_MQTT_USERNAME
#define MQTT_PASS      CONFIG_MQTT_PASSWORD
#define MQTT_TOPIC     CONFIG_MQTT_TOPIC

// I2S configuration
#define I2S_BCK_IO     (GPIO_NUM_47)  // Connect to Amp BCLK
#define I2S_WS_IO      (GPIO_NUM_45)  // Connect to Amp LRC
#define I2S_DO_IO      (GPIO_NUM_21)  // Connect to Amp DIN
#define AUDIO_BUFFER_SIZE 16384
#define RINGBUF_SIZE_KB 64
#define RING_BUFFER_SIZE (RINGBUF_SIZE_KB * 1024)

static EventGroupHandle_t wifi_event_group;
static i2s_chan_handle_t tx_handle = NULL;
static RingbufHandle_t audio_rb = NULL;
static TaskHandle_t i2s_task_handle = NULL;
static volatile bool audio_download_complete = false;

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
    chan_cfg.dma_desc_num = 32;
    chan_cfg.dma_frame_num = 480;
    chan_cfg.auto_clear = true;
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

    // Initial config with default 16kHz - will be reconfigured when playing audio
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
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

static void i2s_write_task(void *pvParameters) {
    size_t item_size;
    size_t bytes_written;
    
    ESP_LOGI(TAG, "I2S writer task started");
    while (1) {
        // Receive data from ring buffer
        void *item = xRingbufferReceive(audio_rb, &item_size, pdMS_TO_TICKS(100));
        if (item) {
            i2s_channel_write(tx_handle, item, item_size, &bytes_written, portMAX_DELAY);
            vRingbufferReturnItem(audio_rb, item);
        } else if (audio_download_complete) {
            // Buffer empty and download finished - double check one last time with 0-wait
            void *last_check = xRingbufferReceive(audio_rb, &item_size, 0);
            if (!last_check) break;
            i2s_channel_write(tx_handle, last_check, item_size, &bytes_written, portMAX_DELAY);
            vRingbufferReturnItem(audio_rb, last_check);
        }
    }
    
    ESP_LOGI(TAG, "I2S writer task finishing (waiting for DMA to drain)...");
    
    // Give enough time for the final ~1s of audio in the DMA buffer to clear
    vTaskDelay(pdMS_TO_TICKS(1200));

    i2s_channel_disable(tx_handle);
    i2s_task_handle = NULL;
    vTaskDelete(NULL);
}

static void audio_playback_task(void *pvParameters) {
    char *url = (char *)pvParameters;
    audio_download_complete = false;

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .buffer_size = 8192,
        .buffer_size_tx = 4096,
        .timeout_ms = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        free(url);
        vTaskDelete(NULL);
        return;
    }

    int content_length = esp_http_client_fetch_headers(client);
    ESP_LOGI(TAG, "Streaming audio (%d bytes)...", content_length);

    // Initial read for WAV header
    uint8_t header[44];
    int r = esp_http_client_read(client, (char*)header, 44);
    if (r != 44) {
        ESP_LOGE(TAG, "Failed to read WAV header");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        free(url);
        vTaskDelete(NULL);
        return;
    }

    uint16_t num_channels = header[22] | (header[23] << 8);
    uint32_t sample_rate = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    uint16_t bits_per_sample = header[34] | (header[35] << 8);
    
    ESP_LOGI(TAG, "WAV: %lu Hz, %u channels, %u bits", (unsigned long)sample_rate, (unsigned)num_channels, (unsigned)bits_per_sample);

    // Disable channel before reconfiguring (ignore state errors)
    esp_err_t dis_err = i2s_channel_disable(tx_handle);
    if (dis_err != ESP_OK && dis_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Unexpected I2S disable error: %s", esp_err_to_name(dis_err));
    }

    // Reconfigure I2S
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate);
    ESP_ERROR_CHECK(i2s_channel_reconfig_std_clock(tx_handle, &clk_cfg));
    i2s_data_bit_width_t bit_width = (bits_per_sample == 16) ? I2S_DATA_BIT_WIDTH_16BIT : I2S_DATA_BIT_WIDTH_32BIT;
    i2s_std_slot_config_t slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(bit_width, I2S_SLOT_MODE_MONO);
    ESP_ERROR_CHECK(i2s_channel_reconfig_std_slot(tx_handle, &slot_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle));

    // Create ring buffer (use a slightly larger buffer for better jitter tolerance)
    audio_rb = xRingbufferCreate(RING_BUFFER_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (!audio_rb) {
        ESP_LOGE(TAG, "Failed to create audio ring buffer!");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        free(url);
        vTaskDelete(NULL);
        return;
    }

    // Wait until we have enough data to start playback (Jitter Buffer)
    const int start_threshold = RING_BUFFER_SIZE / 2; // Start at 50% full
    char *chunk_buffer = malloc(4096);
    char *mono_buffer = malloc(4096);
    int sample_size = bits_per_sample / 8;
    int bytes_processed = 44;
    bool player_started = false;

    ESP_LOGI(TAG, "Buffering...");

    while (1) {
        int read_len = esp_http_client_read(client, chunk_buffer, 4096);
        if (read_len <= 0) break;

        size_t push_len = 0;
        if (num_channels == 2) {
            int frames = read_len / (sample_size * 2);
            if (bits_per_sample == 16) {
                int16_t *src = (int16_t *)chunk_buffer;
                int16_t *dst = (int16_t *)mono_buffer;
                for (int i = 0; i < frames; i++) {
                    dst[i] = src[i * 2];
                }
            } else if (bits_per_sample == 32) {
                int32_t *src = (int32_t *)chunk_buffer;
                int32_t *dst = (int32_t *)mono_buffer;
                for (int i = 0; i < frames; i++) {
                    dst[i] = src[i * 2];
                }
            }
            push_len = frames * sample_size;
            xRingbufferSend(audio_rb, mono_buffer, push_len, portMAX_DELAY);
        } else {
            push_len = read_len;
            xRingbufferSend(audio_rb, chunk_buffer, read_len, portMAX_DELAY);
        }
        bytes_processed += read_len;

        // Start playback once threshold is reached or download is tiny
        if (!player_started) {
            size_t buffered = RING_BUFFER_SIZE - xRingbufferGetCurFreeSize(audio_rb);
            if (buffered >= start_threshold) {
                ESP_LOGI(TAG, "Buffer threshold reached, starting playback");
                xTaskCreate(i2s_write_task, "i2s_task", 4096, NULL, 15, &i2s_task_handle);
                player_started = true;
            }
        }
    }

    // Signal completion
    audio_download_complete = true;
    
    // If download finished but player never started (tiny file), start it now
    if (!player_started) {
        ESP_LOGI(TAG, "Tiny file, starting playback immediately");
        xTaskCreate(i2s_write_task, "i2s_task", 4096, NULL, 15, &i2s_task_handle);
        player_started = true;
    }

    ESP_LOGI(TAG, "Download complete (%d bytes), waiting for writer task to finish...", bytes_processed);

    // Wait for writer task to finish and self-delete
    while (i2s_task_handle != NULL) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vRingbufferDelete(audio_rb);
    audio_rb = NULL;
    free(chunk_buffer);
    free(mono_buffer);
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(url);
    ESP_LOGI(TAG, "Playback task finished.");
    vTaskDelete(NULL);
}

static void play_audio(const char *url) {
    char *url_copy = strdup(url);
    if (url_copy) {
        xTaskCreate(audio_playback_task, "audio_task", 8192, url_copy, 10, NULL);
    }
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
        .broker.verification.crt_bundle_attach = esp_crt_bundle_attach,
        .credentials.username = MQTT_USER,
        .credentials.authentication.password = MQTT_PASS,
    };
    
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "MQTT client started on port 8883");
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    
    ESP_LOGI(TAG, "Starting RemoteAlarm...");
    
    wifi_init();
    i2s_init();
    mqtt_init();
    
    ESP_LOGI(TAG, "RemoteAlarm ready!");
}
