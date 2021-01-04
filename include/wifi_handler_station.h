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
#include "cJSON.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define RETRY_ATTEMPTS 2
// https://serverfault.com/questions/45439/what-is-the-maximum-length-of-a-wifi-access-points-ssid
// https://www.reddit.com/r/homeautomation/comments/cln344/wifi_password_length_limit_on_connected_devices/
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASS_MAX_LENGTH 64
#define WIFI_MAX_STATIONS 10

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

typedef struct wifi_station_info
{
    char ssid[WIFI_SSID_MAX_LENGTH+1];
    char passkey[WIFI_PASS_MAX_LENGTH+1];
} wifi_station_info_t;

wifi_ap_record_t* get_wifi_station_info();
esp_err_t start_wifi_station(char *wifi_station_info_json);
esp_err_t stop_wifi_station();

#endif