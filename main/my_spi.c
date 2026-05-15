#include <stdio.h>
#include "my_spi.h"
#include "esp_log.h"
#include "sdkconfig.h"

spi_device_handle_t spi;

spi_device_handle_t my_spi_setup()
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

uint8_t my_spi_read_register(uint8_t addr)
{
    uint8_t tx_data[2] = {addr | 0x80, 0x00}; // MSB gesetzt für Read
    uint8_t rx_data[2] = {0};

    spi_transaction_t t = {
        .length = 16, // 16 Bits (8 Adresse + 8 Daten)
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    spi_device_polling_transmit(spi, &t);
    return rx_data[1]; // Das zweite Byte enthält den Registerwert
}