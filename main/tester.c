#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <stdio.h>

#include "mqtt_handler.h"
#include "wifi_sta.h"

static const char *TAG = "tester_main";

void app_main(void) {
  ESP_LOGI(TAG, "hello");
  ESP_LOGI(TAG, "world");
  esp_err_t esp_err;

  EventGroupHandle_t network_event_group;
  EventBits_t network_event_bits;
  network_event_group = xEventGroupCreate();

  esp_err = nvs_flash_init();
  if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES ||
      esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(esp_err);

  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  ESP_ERROR_CHECK(wifi_sta_init(network_event_group));

  network_event_bits =
      xEventGroupWaitBits(network_event_group, WIFI_CONNECTED_BIT, pdFALSE,
                          pdTRUE, pdMS_TO_TICKS(5000));
  if (network_event_bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "starting mqtt5");
    mqtt5_start();
  }

  while (1) {
    network_event_bits = xEventGroupGetBits(network_event_group);
    if (network_event_bits & WIFI_CONNECTED_BIT) {
      ESP_LOGI(TAG, "connected to wifi");
    } else {
      ESP_LOGE(TAG, "no connection");
    }

    vTaskDelay(16000 / portTICK_PERIOD_MS);
  }
}
