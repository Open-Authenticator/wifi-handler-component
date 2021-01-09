#include "wifi_handler_access_point.h"

static const char *WIFI_TAG = "wifi_handler_access_point";
static wifi_ap_record_t *wifi_station_array = NULL;
static uint16_t wifi_station_count = 0;
static bool wifi_station_connected_status = false;
static EventGroupHandle_t wifi_event_group; /*!< wifi event group */

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(WIFI_TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);

        wifi_station_connected_status = true;

        xEventGroupSetBits(wifi_event_group, WIFI_STA_CONNECTED_BIT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(WIFI_TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);

        wifi_station_connected_status = false;
    }
}

bool is_wifi_station_connected()
{
    return wifi_station_connected_status;
}

uint16_t wifi_access_point_list_size()
{
    return wifi_station_count;
}

wifi_ap_record_t *scan_wifi_access_point()
{
    if (esp_wifi_scan_start(NULL, true) == ESP_OK)
    {
        if (wifi_station_array != NULL)
        {
            free(wifi_station_array);
            wifi_station_count = 0;
            wifi_station_array = NULL;
        }

        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&wifi_station_count));
        wifi_station_array = calloc(wifi_station_count, sizeof(wifi_ap_record_t));

        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifi_station_count, wifi_station_array));

        return wifi_station_array;
    }

    return NULL;
}

esp_err_t start_wifi_access_point(char *ssid, char *pass)
{
    // create event group for wifi state
    wifi_event_group = xEventGroupCreate();

    wifi_station_connected_status = false;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // create LwIP core task and init LwIP related work.
    ESP_ERROR_CHECK(esp_netif_init());
    // create event loop to handle WiFi related events.
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // create default network interface instance binding station with TCP/IP stack.
    esp_netif_create_default_wifi_ap();

    // create the Wi-Fi driver task and initialize the Wi-Fi driver.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // create instance of event handler, inshort kind of a handle to invoke event handler on receiving certain types of event
    esp_event_handler_instance_t instance_any_id;

    // register events that should be handled by the event handler, so if any wifi event, event handler function will be invoked.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid_len = strlen(ssid),
            .channel = WIFI_CHANNEL,
            .max_connection = WIFI_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    memcpy(wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    memcpy(wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));

    // set wifi mode to station, i.e. connect to other wifi networks
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    // set configuration for AP to be created
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    // start wifi and send event WIFI_EVENT_STA_START to event handler
    ESP_ERROR_CHECK(esp_wifi_start());
    // set wifi power saving mode to max power save
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MAX_MODEM));

    // Wait until some station connects to the access point (WIFI_STA_CONNECTED_BIT) or no station has connected yet (WIFI_STA_DISCONNECTED_BIT).
    // The bits are set by event_handler() (see above)
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_STA_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    // xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened.
    if (bits & WIFI_STA_CONNECTED_BIT)
    {
        ESP_LOGI(WIFI_TAG, "station connected to wifi access point (event_group)");
    }
    else
    {
        ESP_LOGE(WIFI_TAG, "unexpected event");
    }

    // Since, a station has connected successfully to the access point.
    // We can unregister the event handler and return either ESP_OK or ESP_FAIL.
    // The event will not be processed after unregister.
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    // delete wifi event group.
    vEventGroupDelete(wifi_event_group);

    // only if wifi is connected successfully return ESP_OK
    if (bits & WIFI_STA_CONNECTED_BIT)
    {
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t stop_wifi_access_point()
{
    if (wifi_station_array != NULL)
    {
        free(wifi_station_array);
        wifi_station_array = NULL;
    }
    wifi_station_count = 0;
    wifi_station_connected_status = false;

    esp_wifi_stop();
    esp_wifi_deinit();

    ESP_LOGI(WIFI_TAG, "stopped wifi access point");
    return ESP_OK;
}