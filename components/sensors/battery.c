#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "hal/adc_types.h"
#include "mqtt_handler.h"

#include "battery.h"

static const char *TAG = "Battery sensors";

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
