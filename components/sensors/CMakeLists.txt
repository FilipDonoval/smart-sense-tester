idf_component_register(SRCS "battery.c"
                        PRIV_REQUIRES esp_adc mqtt_handler
                        INCLUDE_DIRS "include")
