#pragma once
#include "esp_err.h"
#include <driver/spi_master.h>

// IMU = Intertial Measurement Unit

typedef struct
{
    float x;
    float y;
    float z;
} imu_xyz_t;

void imu_init(spi_device_handle_t device);
esp_err_t imu_test_connection(void);

float imu_read_temperature();
esp_err_t imu_read_acc_data(imu_xyz_t *out_data);
esp_err_t imu_read_gyro_data(imu_xyz_t *out_data);
void imu_teleplot(char *prefix, imu_xyz_t *data);

uint8_t imu_read(uint8_t addr);
esp_err_t imu_read_burst(uint8_t addr, uint8_t *dest, size_t len);
esp_err_t imu_write(uint8_t addr, uint8_t data);