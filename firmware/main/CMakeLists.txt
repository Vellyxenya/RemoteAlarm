idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_client esp_event esp_wifi nvs_flash mqtt driver json)