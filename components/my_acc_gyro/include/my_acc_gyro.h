#pragma once
#include "esp_err.h"
#include <driver/spi_master.h>

typedef struct
{
    float x;
    float y;
    float z;
} my_acc_gyro_xyz_t;

spi_device_handle_t my_acc_gyro_setup();
float my_acc_gyro_read_temperature();
esp_err_t my_acc_gyro_read_acc_data(my_acc_gyro_xyz_t *out_data);
esp_err_t my_acc_gyro_read_gyro_data(my_acc_gyro_xyz_t *out_data);

uint8_t my_acc_gyro_read(uint8_t addr);
esp_err_t my_acc_gyro_read_burst(uint8_t addr, uint8_t *dest, size_t len);
esp_err_t my_acc_gyro_write(uint8_t addr, uint8_t data);