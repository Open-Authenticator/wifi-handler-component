#ifndef WIFI_HANDLER_ACCESS_POINT_H
#define WIFI_HANDLER_ACCESS_POINT_H

#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_CHANNEL 1
#define WIFI_MAX_STA_CONN 1
#define WIFI_SCAN_LIST_SIZE 10
#define WIFI_STA_CONNECTED_BIT BIT0    /*!< used in event group, this bit represents connected bit */

bool is_wifi_station_connected();
uint16_t wifi_access_point_list_size();
wifi_ap_record_t *scan_wifi_access_point();
esp_err_t start_wifi_access_point(char *ssid, char *pass);
esp_err_t stop_wifi_access_point();

#endif