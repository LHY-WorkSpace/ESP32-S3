idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ""
                    REQUIRES  
                            lvgl
                            lvgl_esp32_drivers
                            porting
                            driver
                            Drivers
                            Main_UI
                    )
