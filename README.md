# WiFi Handler component

WiFi handler component for open authenticator. It has capability to handle
connecting to a given list of wifi access points and handle starting an access
point on esp32, to which other devices can connect to.

# Installation

```bash
cd <your_esp_idf_project>
mkdir components
cd components
git clone https://github.com/Open-Authenticator/wifi-handler-component.git wifi_handler
```

# Documentation

# Examples

### Connecting to WiFi station

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_handler_station.h"

void app_main(void)
{
    while (1)
    {
        if(start_wifi_station("{\"c\":2,\"s\":[\"D-Link\",\"Airtel\"],\"p\":[\"yadayada\",\"lololwrong\"]}") == WIFI_ERR_NOT_CONNECTED)
        {
            ESP_LOGE("err", "failed to connect to any wifi");
        }

        wifi_ap_record_t *ap_info = get_wifi_station_info();
        if (ap_info != NULL)
        {
            ESP_LOGI("info", "ssid: %s", ap_info->ssid);
        }
        ESP_LOGI("heap", "heap size: %f", (float)esp_get_free_heap_size() / 1024);

        vTaskDelay(5000 / portTICK_PERIOD_MS);

        stop_wifi_station();

        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }
}
```

### Starting an access point

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_handler_access_point.h"

void app_main(void)
{
    while (1)
    {
        start_wifi_access_point("esp-test", "pass12345");
        
        wifi_ap_record_t *ap_list = scan_wifi_access_point();
        int ap_num = wifi_access_point_list_size();

        if (ap_list != NULL)
        {
            for (int i = 0; i < ap_num; i++)
            {
                ESP_LOGI("ap", "%s", ap_list[i].ssid);
            }
        }

        while (!is_wifi_station_connected());

        vTaskDelay(5000 / portTICK_PERIOD_MS);

        stop_wifi_access_point();

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
```

# License

```
MIT License

Copyright (c) 2020-2021 Vedant Paranjape

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```