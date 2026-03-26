#include "esp_log.h"
#include "mqtt_client.h"
// or

static const char *TAG = "mqtt_handler";

static void mqtt5_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data) {
  ESP_LOGI(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIu32,
           base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;

  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_publish(client, "test", "ovobbbb aaaaa", 0, 1, 1);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    msg_id = esp_mqtt_client_subscribe(client, "test", 0);
    ESP_LOGI(TAG, "subscribe to topic successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;
  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, reason code=0x%02x",
             event->msg_id, (uint8_t)*event->data);
    msg_id = esp_mqtt_client_publish(client, "test", "data", 0, 0, 0);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    esp_mqtt_client_disconnect(client);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
    ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

void mqtt5_start(void) {
  esp_mqtt_client_config_t mqtt5_client = {
      .broker.address.uri = CONFIG_MQTT_BROKER_URI,
      .credentials.username = CONFIG_MQTT_BROKER_USERNAME,
      .credentials.authentication.password = CONFIG_MQTT_BROKER_PASSWORD,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt5_client);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt5_event_handler,
                                 client);
  esp_mqtt_client_start(client);
}
