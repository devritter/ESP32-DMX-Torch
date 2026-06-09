idf_component_register(SRCS "teleplot.c" "buttons.c" "main.c" "imu.c" "dmx.c" "movinghead.c" "my_spi.c" "utils.c" "display.c" "screen_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_i2c esp_driver_gpio esp_driver_uart my_led_matrix)
