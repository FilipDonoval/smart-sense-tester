idf_component_register(SRCS "tester.c"
                    PRIV_REQUIRES nvs_flash wifi_sta mqtt_handler sensors
                    INCLUDE_DIRS ".")
