#include "a7670_modem.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_modem_api.h"
#include "esp_modem_c_api_types.h"
#include "esp_netif_ppp.h"

static const char *TAG = "a7670_modem";
static EventGroupHandle_t modem_event_group = NULL;

static void on_ip_event(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "IP event %" PRIu32, event_id);

  if (event_id == IP_EVENT_PPP_GOT_IP) {
    esp_netif_dns_info_t dns_info;

    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    esp_netif_t *netif = event->esp_netif;

    ESP_LOGI(TAG, "Modem connected to PPP server");
    ESP_LOGI(TAG, "IP: " IPSTR, IP2STR(&event->ip_info.ip));

    xEventGroupSetBits(modem_event_group, CONNECTED_BIT);

  } else if (event_id == IP_EVENT_PPP_LOST_IP) {
    ESP_LOGI(TAG, "Modem disconnected from PPP server");
    xEventGroupClearBits(modem_event_group, CONNECTED_BIT);
  } else if (event_id == IP_EVENT_GOT_IP6) {
    ESP_LOGI(TAG, "Got IPv6 event");
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    ESP_LOGI(TAG, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
  }
}

static void on_ppp_changed(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "PPP state changed event %" PRIu32, event_id);
  if (event_id == NETIF_PPP_ERRORUSER) {
    esp_netif_t **p_netif = event_data;
    ESP_LOGI(TAG, "User interrupted event from netif:%p", *p_netif);
  }
}

void turn_on_modem() {

  gpio_reset_pin(PERIPHERAL_POWER_CONTROL);
  gpio_set_direction(PERIPHERAL_POWER_CONTROL, GPIO_MODE_OUTPUT);
  gpio_set_level(PERIPHERAL_POWER_CONTROL, 1);

  gpio_reset_pin(PWR);
  gpio_set_direction(PWR, GPIO_MODE_OUTPUT);
  gpio_set_level(PWR, 0);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(PWR, 1);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  gpio_set_level(PWR, 0);

  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

void modem_setup() {

  modem_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID,
                                             &on_ppp_changed, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                             &on_ip_event, NULL));

  turn_on_modem();

  esp_modem_dce_config_t dce_config = ESP_MODEM_DCE_DEFAULT_CONFIG("test apn");
  esp_netif_config_t netif_ppp_config = ESP_NETIF_DEFAULT_PPP();
  esp_netif_t *esp_netif = esp_netif_new(&netif_ppp_config);

  esp_modem_dte_config_t dte_config = ESP_MODEM_DTE_DEFAULT_CONFIG();

  dte_config.uart_config.tx_io_num = TX;
  dte_config.uart_config.rx_io_num = RX;
  dte_config.uart_config.rts_io_num = RESET;

  esp_modem_dce_t *dce = esp_modem_new_dev(ESP_MODEM_DCE_SIM7600, &dte_config,
                                           &dce_config, esp_netif);
  esp_err_t esp_err;
  esp_err = esp_modem_sync(dce);

  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "esp_modem_sync failed with %d", esp_err);
  }

  // TEST MODEM CONNECTION
  esp_err = esp_modem_set_mode(dce, ESP_MODEM_MODE_DETECT);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "esp_modem_set_mode ESP_MODEM_MODE_DETECT failed with %d",
             esp_err);
    return;
  }

  esp_modem_dce_mode_t mode = esp_modem_get_mode(dce);
  ESP_LOGI(TAG, "Current mode is: %d", mode);
  if (mode == ESP_MODEM_MODE_DATA) {
    esp_err = esp_modem_set_mode(dce, ESP_MODEM_MODE_COMMAND);
    if (esp_err != ESP_OK) {
      ESP_LOGE(TAG, "esp_modem_set_mode(ESP_MODEM_MODE_COMMAND) failed with %d",
               esp_err);
      return;
    }
    ESP_LOGI(TAG, "Command mode restored");
  }
  // END TEST MODEM CONNECTION
}
