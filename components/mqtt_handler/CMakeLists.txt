idf_component_register(SRCS "mqtt_handler.c"
                    PRIV_REQUIRES esp_wifi nvs_flash mqtt
                    INCLUDE_DIRS "include")
