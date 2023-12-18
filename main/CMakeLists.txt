idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "./"
                    REQUIRES  
                            lvgl
                            lvgl_esp32_drivers
                            porting
                            driver
                            Drivers
                            Main_UI
                            UI
                            LED
                            esp_http_server
                            esp_http_client
                            esp_wifi
                            esp_adc
                            sdmmc
                            fatfs
                            espressif_esp32_camera
                    )
