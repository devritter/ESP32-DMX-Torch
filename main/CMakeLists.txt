idf_component_register(SRCS "main.c" "my_dmx.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_gpio esp_driver_uart my_led_matrix my_acc_gyro)