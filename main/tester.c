#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "portmacro.h"
#include "sdkconfig.h"
#include <stdio.h>

#define MAX_RECONNECT_RETRY_NUMBER 10

static EventGroupHandle_t s_wifi_event_group = NULL;

#define WIFI_CONNECTED_BIT BIT0
static int s_retry_number = 0;

static const char *TAG = "tester";

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_number < MAX_RECONNECT_RETRY_NUMBER) {
      esp_wifi_connect();
      s_retry_number++;
      ESP_LOGI(TAG, "retry to connect to AP");
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_number = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void wifi_init_sta(void) {
  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

  // Configure minimum WiFi authentication mode
#if CONFIG_WIFI_STA_AUTH_OPEN
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_OPEN;
#elif CONFIG_WIFI_STA_AUTH_WEP
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WEP;
#elif CONFIG_WIFI_STA_AUTH_WPA_PSK
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA_PSK;
#elif CONFIG_WIFI_STA_AUTH_WPA2_PSK
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA2_PSK;
#elif CONFIG_WIFI_STA_AUTH_WPA_WPA2_PSK
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA_WPA2_PSK;
#elif CONFIG_WIFI_STA_AUTH_WPA3_PSK
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA3_PSK;
#elif CONFIG_WIFI_STA_AUTH_WPA2_WPA3_PSK
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WPA2_WPA3_PSK;
#elif CONFIG_WIFI_STA_AUTH_WAPI_PSK
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_WAPI_PSK;
#else
  const wifi_auth_mode_t auth_mode = WIFI_AUTH_OPEN;
#endif

  // Configure WPA3 SAE mode
#if CONFIG_WIFI_STA_WPA3_SAE_PWE_HUNT_AND_PECK
  const wifi_sae_pwe_method_t sae_pwe_method = WPA3_SAE_PWE_HUNT_AND_PECK;
#elif CONFIG_WIFI_STA_WPA3_SAE_PWE_H2E
  const wifi_sae_pwe_method_t sae_pwe_method = WPA3_SAE_PWE_HASH_TO_ELEMENT;
#elif CONFIG_WIFI_STA_WPA3_SAE_PWE_BOTH
  const wifi_sae_pwe_method_t sae_pwe_method = WPA3_SAE_PWE_BOTH;
#else
  const wifi_sae_pwe_method_t sae_pwe_method = WPA3_SAE_PWE_UNSPECIFIED;
#endif

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = CONFIG_WIFI_STA_SSID,
              .password = CONFIG_WIFI_STA_PASSWORD,
              .threshold.authmode = auth_mode,
              .sae_pwe_h2e = sae_pwe_method,
              .sae_h2e_identifier = CONFIG_WIFI_STA_WPA3_PASSWORD_ID,
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "wifi_init_sta finished.");

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                                         pdFALSE, pdTRUE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID:%s", CONFIG_WIFI_STA_SSID);
  } else {
    ESP_LOGI(TAG, "failed to connect to network");
  }
}

void app_main(void) {
  ESP_LOGI(TAG, "hello");
  ESP_LOGI(TAG, "world");

  esp_err_t esp_err = nvs_flash_init();
  if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES ||
      esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(esp_err);

  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  wifi_init_sta();

  EventBits_t wifi_bits = xEventGroupWaitBits(
      s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

  while (1) {
    if (wifi_bits & WIFI_CONNECTED_BIT) {
      ESP_LOGI(TAG, "still connected");
    } else {
      ESP_LOGE(TAG, "lost connection");
      abort();
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
