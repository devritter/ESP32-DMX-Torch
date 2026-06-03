#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "imu.h"
#include "esp_log.h"
#include "utils.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#define START_ADDR_POWER_MGMT 0x4E
#define START_ADDR_WHO_AM_I 0x75
#define START_ADDR_TEMP 0x1D
#define START_ADDR_ACCE 0x1F
#define START_ADDR_GYRO 0x25

static spi_device_handle_t spi;
static esp_err_t read_xyz_internal(imu_xyz_t *out_data, uint8_t start_addr, float scale_factor);

void imu_init(spi_device_handle_t device)
{
    spi = device;
    // reset device
    // my_acc_gyro_write(0x11, 0x01);

    ESP_ERROR_CHECK(imu_write(START_ADDR_POWER_MGMT, 0b00001111)); // 0x4E = Power Management

    // 200us no writes, 45ms for the gyro
    sleep_ms(50);
}

esp_err_t imu_test_connection()
{
    uint8_t who_am_i = imu_read(START_ADDR_WHO_AM_I);
    printf("WHO_AM_I = %d (expected: %d)\n", who_am_i, 0x47);

    if (who_am_i == 0x47)
    {
        return ESP_OK;
    }

    return ESP_ERR_INVALID_RESPONSE;
}

float imu_read_temperature()
{
    uint8_t temp_high = imu_read(START_ADDR_TEMP);
    uint8_t temp_low = imu_read(START_ADDR_TEMP + 1);
    int16_t temp_raw = temp_high << 8 | temp_low;
    float temp_centidegree = ((float)temp_raw / 132.48f) + 25.0f;
    return temp_centidegree;
}

esp_err_t imu_read_acc_data(imu_xyz_t *out_data)
{
    return read_xyz_internal(out_data, START_ADDR_ACCE, 2048.0f);
}

esp_err_t imu_read_gyro_data(imu_xyz_t *out_data)
{
    return read_xyz_internal(out_data, START_ADDR_GYRO, 1);
}

uint8_t imu_read(uint8_t addr)
{
    // for reads, MSB must be 1 (| 0x80 to ensure that)
    uint8_t tx_data[2] = {addr | 0x80, 0x00};
    uint8_t rx_data[2] = {0};

    spi_transaction_t t = {
        .length = 16, // 16 Bits (8 Adresse + 8 Daten)
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
    return rx_data[1]; // Das zweite Byte enthält den Registerwert
}

esp_err_t imu_read_burst(uint8_t addr, uint8_t *dest, size_t len)
{
    if (dest == NULL || len == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    size_t total_bytes = 1 + len; // addr + len

    uint8_t *tx_data = calloc(1, total_bytes);
    uint8_t *rx_data = malloc(total_bytes);

    tx_data[0] = addr | 0x80;
    spi_transaction_t t = {
        .length = total_bytes * 8, // length in bits
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    esp_err_t ret = spi_device_polling_transmit(spi, &t);

    if (ret == ESP_OK)
    {
        // Die Daten starten ab Index 1 (Index 0 war die Adressphase)
        memcpy(dest, &rx_data[1], len);
    }

    free(tx_data);
    free(rx_data);

    return ret;
}

static void check_clipping(int16_t value)
{
    if (value == INT16_MIN || value == INT16_MAX)
    {
        printf("!!! IMU sensor clipped value !!!\n");
    }
}

static esp_err_t read_xyz_internal(imu_xyz_t *out_data, uint8_t start_addr, float scale_factor)
{
    uint8_t data[6];
    esp_err_t ret = imu_read_burst(start_addr, data, LEN(data));
    if (ret != ESP_OK)
    {
        return ret;
    }

    // 1. Rohdaten sind strikt 16-Bit vorzeichenbehaftet
    int16_t raw_x = (int16_t)((data[0] << 8) | data[1]);
    int16_t raw_y = (int16_t)((data[2] << 8) | data[3]);
    int16_t raw_z = (int16_t)((data[4] << 8) | data[5]);

    check_clipping(raw_x);
    check_clipping(raw_y);
    check_clipping(raw_z);

    out_data->x = raw_x / scale_factor;
    out_data->y = raw_y / scale_factor;
    out_data->z = raw_z / scale_factor;

    return ESP_OK;
}

esp_err_t imu_write(uint8_t addr, uint8_t data)
{
    // for writes, MSB must be 0 (& 0x7F to ensure that)
    uint8_t tx_data[2] = {addr & 0x7F, data};

    spi_transaction_t t = {
        .length = 16, // 16 Bits insgesamt (8 Bit Adresse, 8 Bit Daten)
        .tx_buffer = tx_data,
        .rx_buffer = NULL // Wir erwarten keine Rückgabedaten beim Schreiben
    };

    return spi_device_polling_transmit(spi, &t);
}

void imu_teleplot(char *prefix, imu_xyz_t *data)
{
    printf(">%sx:%f\n", prefix, data->x);
    printf(">%sy:%f\n", prefix, data->y);
    printf(">%sz:%f\n", prefix, data->z);
}