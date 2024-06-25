file(GLOB_RECURSE SOURCES "./*.c" )

idf_component_register( SRCS	${SOURCES}
                        INCLUDE_DIRS "./"
                        REQUIRES    
                            driver
                            lvgl
                            Main_UI
                            esp_http_server
                            esp_http_client
                            esp_wifi
                            esp_adc
                            nvs_flash
                            LED
                            sdmmc
                            fatfs
                            espressif_esp32_camera
                            wpa_supplicant
                            esp_https_ota
                            app_update
                            spiffs
                            )

spiffs_create_partition_image(storage ./data FLASH_IN_PROJECT)


                        