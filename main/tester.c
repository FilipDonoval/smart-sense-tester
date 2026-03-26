#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "hal/adc_types.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <stdio.h>

#include "mqtt_handler.h"
#include "wifi_sta.h"

static const char *TAG = "tester_main";

#define BOARD_BAT_ADC_PIN 35
void read_battery_stats(void *pvParameters) {
  int battery_read;
  float batter_voltage = 0;
  char msg[20];

  adc_oneshot_unit_handle_t handle = NULL;
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &handle));

  adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12,
  };

  ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, ADC_CHANNEL_7, &config));

  adc_cali_handle_t cali_handle = NULL;

  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
  };

  ESP_ERROR_CHECK(
      adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle));

  while (1) {
    ESP_ERROR_CHECK(adc_oneshot_read(handle, ADC_CHANNEL_7, &battery_read));
    batter_voltage = (battery_read / 4095.0) * 2.0 * 3.3;
    ESP_LOGI(TAG, "raw read: %d, calculated: %f", battery_read, batter_voltage);

    snprintf(msg, sizeof(msg), "%f", batter_voltage);

    mqtt5_publish("Lilygo/sensors/battery/voltage", msg);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

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

  xTaskCreatePinnedToCore(read_battery_stats, "read battery stats", 4096, NULL,
                          10, NULL, 0);

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
