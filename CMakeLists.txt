idf_component_register(SRCS "src/wifi_handler_station.c" "src/wifi_handler_access_point.c"
                       INCLUDE_DIRS "include"
                       REQUIRES efuse esp32 esp_common esp_event 
                                esp_rom freertos log soc nvs_flash json)
