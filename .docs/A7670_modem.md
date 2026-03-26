# LilyGo A7670E
## Required configuration
- When using SD or modem GPIO Peripheral power control pin (for Lilygo a7670e: pin 12) must be set to high level, otherwise these functions cannot be used.

# Libraries
## ESP-MODEM
- For a7670 modem with esp-modem Espressif says that it is similar enough to a7600 that ESP_MODEM_DCE_SIM7600 can be used

- esp-modem manual  
https://docs.espressif.com/projects/esp-protocols/esp_modem/docs/latest/index.html

- esp-modem intro  
https://docs.espressif.com/projects/esp-protocols/esp_modem/docs/latest/README.html

- c interface  
https://docs.espressif.com/projects/esp-protocols/esp_modem/docs/latest/api_docs.html

- c++ interface  
https://docs.espressif.com/projects/esp-protocols/esp_modem/docs/latest/cxx_api_docs.html

- dce internal implementation  
https://docs.espressif.com/projects/esp-protocols/esp_modem/docs/latest/internal_docs.html

- clien example  
https://github.com/espressif/esp-protocols/blob/ca6e2835fe53bbe0996d8f5fcbcd539a86f4bb47/components/esp_modem/examples/pppos_client/main/pppos_client_main.c


