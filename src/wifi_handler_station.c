#include "wifi_handler_station.h"

static const char *WIFI_TAG = "wifi_handler_station";
static int retry_count = 0;
static int station_count = 0;
static wifi_station_info_t *wifi_station_array = NULL;
static int wifi_station_array_index = 0;
static EventGroupHandle_t wifi_event_group;
static wifi_ap_record_t connected_station_info;

static esp_err_t parse_wifi_station_info_json(const char *wifi_station_info_json)
{
    station_count = 0;
    if (wifi_station_array != NULL)
    {
        free(wifi_station_array);
        wifi_station_array = NULL;
    }

    cJSON *root = cJSON_Parse(wifi_station_info_json);
    if (root == NULL)
    {
        return ESP_FAIL;
    }

    if (cJSON_HasObjectItem(root, "c"))
    {
        station_count = cJSON_GetObjectItem(root, "c")->valueint;
        station_count = station_count <= WIFI_MAX_STATIONS ? station_count : 5;
        station_count = station_count >= 0 ? station_count : 0;
    }
    else
    {
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    wifi_station_array = calloc(station_count, sizeof(wifi_station_info_t));

    cJSON *ssid_array = NULL, *pass_array = NULL;
    cJSON *ssid_iter = NULL, *pass_iter = NULL;
    if (cJSON_HasObjectItem(root, "s") && cJSON_HasObjectItem(root, "p"))
    {
        ssid_array = cJSON_GetObjectItem(root, "s");
        pass_array = cJSON_GetObjectItem(root, "p");

        if (cJSON_GetArraySize(ssid_array) != station_count && cJSON_GetArraySize(pass_array) != station_count)
        {
            free(wifi_station_array);
            cJSON_Delete(root);
            return ESP_FAIL;
        }

        ssid_iter = ssid_array->child;
        pass_iter = pass_array->child;
    }
    else
    {
        free(wifi_station_array);
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    for (int i = 0; (ssid_iter != NULL) && (pass_iter != NULL) && (i < station_count); ssid_iter = ssid_iter->next, pass_iter = pass_iter->next, i++)
    {
        if (cJSON_IsString(ssid_iter) && cJSON_IsString(pass_iter))
        {
            strncpy(wifi_station_array[i].ssid, ssid_iter->valuestring, WIFI_SSID_MAX_LENGTH + 1);
            strncpy(wifi_station_array[i].passkey, pass_iter->valuestring, WIFI_PASS_MAX_LENGTH + 1);
        }
        else
        {
            free(wifi_station_array);
            cJSON_Delete(root);
            return ESP_FAIL;
        }
    }

    cJSON_Delete(root);
    return ESP_OK;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(WIFI_TAG, "connecting to wifi ssid: %s", wifi_station_array[wifi_station_array_index].ssid);

        wifi_config_t wifi_config = {
            .sta = {
                .threshold.authmode = WIFI_AUTH_OPEN,
                .pmf_cfg = {
                    .capable = true,
                    .required = false},
            },
        };
        memcpy(wifi_config.sta.ssid, wifi_station_array[wifi_station_array_index].ssid, WIFI_SSID_MAX_LENGTH);
        memcpy(wifi_config.sta.password, wifi_station_array[wifi_station_array_index].passkey, WIFI_PASS_MAX_LENGTH);
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

        // connect to wifi, since wifi driver was setup correctly
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(WIFI_TAG, "connected to wifi ssid (event_handler): %s", wifi_station_array[wifi_station_array_index].ssid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // if we retried less than the retry attempts count, then retry to connect, else load new ssid from wifi_station_array
        if (retry_count < RETRY_ATTEMPTS)
        {
            // increment retry_count
            retry_count++;
            ESP_LOGI(WIFI_TAG, "connecting to wifi (retry %d)", retry_count);
            // retry connecting to wifi
            esp_wifi_connect();
        }
        else
        {
            retry_count = 0;
            wifi_station_array_index++;

            if (wifi_station_array_index < station_count)
            {
                ESP_LOGI(WIFI_TAG, "connecting to wifi ssid: %s", wifi_station_array[wifi_station_array_index].ssid);

                wifi_config_t wifi_config = {
                    .sta = {
                        .threshold.authmode = WIFI_AUTH_OPEN,
                        .pmf_cfg = {
                            .capable = true,
                            .required = false},
                    },
                };
                memcpy(wifi_config.sta.ssid, wifi_station_array[wifi_station_array_index].ssid, WIFI_SSID_MAX_LENGTH);
                memcpy(wifi_config.sta.password, wifi_station_array[wifi_station_array_index].passkey, WIFI_PASS_MAX_LENGTH);
                ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

                // connect to wifi, since wifi driver was setup correctly
                esp_wifi_connect();
            }
            else
            {
                // set wifi fail event group bit
                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
            }
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

        retry_count = 0;
        wifi_station_array_index = 0;
        // set wifi connected event group bit
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

wifi_ap_record_t* get_wifi_station_info()
{    
    if(esp_wifi_sta_get_ap_info(&connected_station_info) == ESP_OK)
    {
        return &connected_station_info;
    }

    return NULL;
}

/**
 * wifi_station_info_json --> json structure is as follows:
 * 
 * {
 *    "c": 3 (max 10, int, set by macro WIFI_MAX_STATIONS),
 *    "s": ["hello", "bye", "df"],
 *    "p": ["fakee", "nice", "ddddfs"]
 * }
 * 
 * these ssid and pass are corresponding according to their array location, i.e pass of ssid[0] = pass[0] and so on
 **/
esp_err_t start_wifi_station(char *wifi_station_info_json)
{
    // create event group for wifi state
    wifi_event_group = xEventGroupCreate();

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // !!! check for length of wifi_station_info_json, it should not cross a specific length. possibly use strncpy
    // copy the json string containing wifi station to try to connect to
    char *wifi_station_info_json_ = calloc(strlen(wifi_station_info_json) + 1, sizeof(char));
    strcpy(wifi_station_info_json_, wifi_station_info_json);
    ESP_ERROR_CHECK_WITHOUT_ABORT(parse_wifi_station_info_json((const char *)wifi_station_info_json_));
    free(wifi_station_info_json_);

    // create LwIP core task and init LwIP related work.
    ESP_ERROR_CHECK(esp_netif_init());
    // create event loop to handle WiFi related events.
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // create default network interface instance binding station with TCP/IP stack.
    esp_netif_create_default_wifi_sta();

    // create the Wi-Fi driver task and initialize the Wi-Fi driver.
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // create instance of event handler, inshort kind of a handle to invoke event handler on receiving certain types of event
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    // register events that should be handled by the event handler, so if any wifi event or IP_EVENT_STA_GOT_IP is raised, event handler
    // function will be invoked.
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

    // set wifi mode to station, i.e. connect to other wifi networks
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // start wifi and send event WIFI_EVENT_STA_START to event handler
    ESP_ERROR_CHECK(esp_wifi_start());
    // set wifi power saving mode to max power save
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MAX_MODEM));

    // Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    // number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above)
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    // xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually happened.
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(WIFI_TAG, "connected to wifi ssid (event_group): %s", wifi_station_array[wifi_station_array_index].ssid);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(WIFI_TAG, "Failed to connect to any wifi stations from ssid list passed to start_wifi_station()");
    }
    else
    {
        ESP_LOGE(WIFI_TAG, "unexpected event");
    }

    // Since, we have successfully connected or failed to connect to wifi after all attempts.
    // We can unregister the event handler and return either ESP_OK or ESP_FAIL.
    // The event will not be processed after unregister.
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));

    // only if wifi is connected successfully return ESP_OK
    if (bits & WIFI_CONNECTED_BIT)
    {
        vEventGroupDelete(wifi_event_group);
        return ESP_OK;
    }

    // delete wifi event group.
    vEventGroupDelete(wifi_event_group);
    return ESP_FAIL;
}

esp_err_t stop_wifi_station()
{
    free(wifi_station_array);
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();

    ESP_LOGI(WIFI_TAG, "disconnected from wifi");
    return ESP_OK;
}
