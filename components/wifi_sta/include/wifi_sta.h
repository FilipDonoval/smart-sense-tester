#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"

#define MAX_RECONNECT_RETRY_NUMBER 10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_STA_IPV4_OBRAINED_BIT BIT1
esp_err_t wifi_sta_init(EventGroupHandle_t event_group);

#endif
