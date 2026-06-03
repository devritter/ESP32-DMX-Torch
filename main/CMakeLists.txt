idf_component_register(SRCS "imu.c" "main.c" "dmx.c" "movinghead.c" "imu.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_gpio esp_driver_uart my_led_matrix)
