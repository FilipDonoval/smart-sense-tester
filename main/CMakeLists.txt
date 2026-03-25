idf_component_register(SRCS "tester.c"
                    PRIV_REQUIRES esp_wifi nvs_flash wifi_sta
                    INCLUDE_DIRS ".")
