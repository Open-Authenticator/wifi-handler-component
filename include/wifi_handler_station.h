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

#define WIFI_RECONNECT_RETRY_ATTEMPTS 2        /*!< number of times to try to reconnect to same wifi ssid */
#define WIFI_SSID_MAX_LENGTH 32                /*!< max length of ssid name (https://serverfault.com/questions/45439/what-is-the-maximum-length-of-a-wifi-access-points-ssid) */
#define WIFI_PASS_MAX_LENGTH 64                /*!< max length of password (https://www.reddit.com/r/homeautomation/comments/cln344/wifi_password_length_limit_on_connected_devices/) */
#define WIFI_MAX_STATIONS 10                   /*!< max number of wifi stations to try to connect */
#define WIFI_MAX_STATION_INFO_STRING_SIZE 1040 /*!< max size of station info string */
#define WIFI_CONNECTED_BIT BIT0                /*!< used in event group, this bit represents connected bit */
#define WIFI_FAIL_BIT BIT1                     /*!< used in event group, this bit represents the disconnected bit */
#define WIFI_ERR_NOT_CONNECTED -2              /*!< error code if wifi failed to connect to any of the stored networks */
#define WIFI_ERR_ALREADY_RUNNING -3            /*!< error code if wifi is already running and `start_wifi_station()` is called */ 
#define WIFI_ERR_STA_INFO -4                   /*!< error code if wifi_station_info_json passed is invalid or larger than expected value */

/**
 * @brief stores information about wifi access point
 */
typedef struct wifi_station_info
{
    char ssid[WIFI_SSID_MAX_LENGTH + 1];    /**< wifi ssid name */
    char passkey[WIFI_PASS_MAX_LENGTH + 1]; /**< wifi password */
} wifi_station_info_t;

/**
 * @brief Gets the information about the currently connected access point
 * 
 * @return wifi_ap_record_t* returns pointer to a static instance of wifi_ap_record_t, if esp32 is not connected to any access point, it return NULL
 */
wifi_ap_record_t *get_wifi_station_info();

/**
 * @brief starts wifi and connects to access points which are passed as json string to this function
 * 
 * wifi_station_info_json --> json structure is as follows:
 * 
 * {
 *    "c": 3 (max 10, int, set by macro WIFI_MAX_STATIONS),
 *    "s": ["hello", "bye", "df"],
 *    "p": ["fakee", "nice", "ddddfs"]
 * }
 * 
 * these ssid and pass are corresponding according to their array location, i.e pass of ssid[0] = pass[0] and so on
 * 
 * Flow of this function is as follows:
 * 1) Tries to connect to the first wifi AP specified in the json string
 * 1.1) If connected successfully return ESP_OK 
 * 1.2) If fails, then tries to reconnect, keeps on trying WIFI_RECONNECT_RETRY_ATTEMPTS times
 * 1.2.1) If connected successfully return ESP_OK
 * 1.2.2) If fails after retrying WIFI_RECONNECT_RETRY_ATTEMPTS times, fetches new wifi ssid from json string and goes back to step 1) 
 * 2) If it failed to connect to all wifi APs from the json string return ESP_FAIL
 * 
 * @param wifi_station_info_json json string which contains information about number of AP to which to try to connect to, and also their ssid and passwords
 * @return esp_err_t ESP_OK if connected successfully, WIFI_ERR_ALREADY_RUNNING
 * if wifi is already running, WIFI_ERR_STA_INFO if the given station info is
 * incorrect, WIFI_ERR_NOT_CONNECTED if it couldn't connect to any wifi network
 * given in the list
 */
esp_err_t start_wifi_station(char *wifi_station_info_json);

/**
 * @brief disconnects from currently connected AP and turns off wifi
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t stop_wifi_station();

#endif