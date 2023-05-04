set(srcs "src/led_strip_api.c")

list(APPEND srcs "src/led_strip_rmt_dev.c" "src/led_strip_rmt_encoder.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include" "interface"
                       PRIV_REQUIRES "driver")
