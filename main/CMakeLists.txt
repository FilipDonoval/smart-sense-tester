idf_component_register(SRCS "tester.c"
                    PRIV_REQUIRES nvs_flash wifi_sta mqtt_handler sensors esp_driver_gpio a7670_modem
                    INCLUDE_DIRS ".")
