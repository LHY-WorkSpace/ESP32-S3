file(GLOB_RECURSE SOURCES "./*.c" )

idf_component_register( SRCS	${SOURCES}
                        INCLUDE_DIRS "./"
                        REQUIRES    
                            driver
                            lvgl
                            esp_http_server
                            esp_wifi
                            )




                        