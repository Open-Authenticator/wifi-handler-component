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
#define WIFI_ERR_NOT_CONNECTED -2      /*!< error code if no device is connected to wifi AP */
#define WIFI_ERR_ALREADY_RUNNING -3    /*!< error code if access points is already running and `start_wifi_access_point()` is called */

/**
 * @brief  Tells if any wifi station is connected to wifi access point
 * 
 * @return true - If station is connected
 * @return false - If no station is connected
 */
bool is_wifi_station_connected();

/**
 * @brief Returns size of the list containing scanned access points available
 * 
 * @return uint16_t - size of list
 */
uint16_t wifi_access_point_list_size();

/**
 * @brief Scans wifi access points and returns the struct containing the access
 * points found nearby
 * 
 * @return wifi_ap_record_t* - Returns NULL if failed, or else returns array
 * containing wifi access point list 
 */
wifi_ap_record_t *scan_wifi_access_point();

/**
 * @brief Starts wifi access point so that other devices can connect to esp32
 * AP. Only one device can be connected to this, can be changed by changing the
 * macro `WIFI_MAX_STA_CONN`.
 * 
 * @param ssid string which contains the name of the ssid of the access point
 * started by esp32
 * @param pass string which contains the password of the ssid of the access
 * point started by esp32
 * @return esp_err_t ESP_OK if access points starts correctly and device connects to 
 * access point successfully, WIFI_ERR_ALREADY_RUNNING if access point is already
 * working, WIFI_ERR_NOT_CONNECTED if no device connected to the access point.
 */
esp_err_t start_wifi_access_point(char *ssid, char *pass);

/**
 * @brief Turns off the access point and turns off wifi
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t stop_wifi_access_point();

#endif