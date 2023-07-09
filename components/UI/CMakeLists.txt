file(GLOB_RECURSE SOURCES "./*.c"
                          "screens/*.c" 
                          "fonts/*.c"
                            )

idf_component_register(SRCS	${SOURCES}
                   INCLUDE_DIRS "./"
                    REQUIRES    
                            lvgl
                            lvgl_esp32_drivers
                            Main_UI
                            )
