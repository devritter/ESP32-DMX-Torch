#include <stdio.h>
#include "my_acc_gyro.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#define START_ADDR_TEMP 0x1D
#define START_ADDR_ACCE 0x1F
#define START_ADDR_GYRO 0x25

spi_device_handle_t spi;

spi_device_handle_t my_acc_gyro_setup()
{
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = 4,
        .miso_io_num = 7,
        .sclk_io_num = 10,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0 // DMA default
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz
        .mode = 0,
        .spics_io_num = 1, // Pin13 = IO1
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    return ESP_OK;
}

float my_acc_gyro_read_temperature()
{
    uint8_t temp_high = my_acc_gyro_read(START_ADDR_TEMP);
    uint8_t temp_low = my_acc_gyro_read(START_ADDR_TEMP + 1);
    int16_t temp_raw = temp_high << 8 | temp_low;
    float temp_centidegree = ((float)temp_raw / 132.48f) + 25.0f;
    return temp_centidegree;
}

esp_err_t read_xyz_internal(my_acc_gyro_xyz_t *out_data, uint8_t start_addr, float scale_factor);

esp_err_t my_acc_gyro_read_acc_data(my_acc_gyro_xyz_t *out_data)
{
    return read_xyz_internal(out_data, START_ADDR_ACCE, 2048.0f);
}

esp_err_t my_acc_gyro_read_gyro_data(my_acc_gyro_xyz_t *out_data)
{
    return read_xyz_internal(out_data, START_ADDR_GYRO, 1);
}

uint8_t my_acc_gyro_read(uint8_t addr)
{
    // for writes, MSB must be 1 (| 0x80 to ensure that)
    uint8_t tx_data[2] = {addr | 0x80, 0x00};
    uint8_t rx_data[2] = {0};

    spi_transaction_t t = {
        .length = 16, // 16 Bits (8 Adresse + 8 Daten)
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    spi_device_polling_transmit(spi, &t);
    return rx_data[1]; // Das zweite Byte enthält den Registerwert
}

esp_err_t my_acc_gyro_read_burst(uint8_t addr, uint8_t *dest, size_t len)
{
    if (dest == NULL || len == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    size_t total_bytes = 1 + len; // add + len

    uint8_t *tx_data = calloc(1, total_bytes);
    uint8_t *rx_data = malloc(total_bytes);

    tx_data[0] = addr | 0x80;
    spi_transaction_t t = {
        .length = total_bytes * 8, // Länge in Bits
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

esp_err_t read_xyz_internal(my_acc_gyro_xyz_t *out_data, uint8_t start_addr, float scale_factor)
{
    uint8_t data[6];
    esp_err_t ret = my_acc_gyro_read_burst(start_addr, data, LEN(data));
    if (ret != ESP_OK)
    {
        return ret;
    }

    // 1. Rohdaten sind strikt 16-Bit vorzeichenbehaftet
    int16_t raw_x = (int16_t)((data[0] << 8) | data[1]);
    int16_t raw_y = (int16_t)((data[2] << 8) | data[3]);
    int16_t raw_z = (int16_t)((data[4] << 8) | data[5]);

    // 2. Berechnung in 32-Bit casten, um Überlauf bei * 1000 zu verhindern
    out_data->x = raw_x / scale_factor;
    out_data->y = raw_y / scale_factor;
    out_data->z = raw_z / scale_factor;

    return ESP_OK;
}

esp_err_t my_acc_gyro_write(uint8_t addr, uint8_t data)
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