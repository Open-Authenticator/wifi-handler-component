file(GLOB srcs RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include"
                       REQUIRES efuse esp32 esp_common esp_event 
                                esp_rom freertos heap log soc spiffs)
