idf_component_register(SRCS "main.c" "my_spi.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_gpio my_led_matrix)
