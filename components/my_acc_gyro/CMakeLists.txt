idf_component_register(SRCS "my_acc_gyro.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_spi esp_driver_gpio)
