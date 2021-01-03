#ifndef WIFI_HANDLER_STATION_H
#define WIFI_HANDLER_STATION_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

esp_err_t start_wifi_station(char *ssid, char *pass);
esp_err_t stop_wifi_station();
#endif