#include "my_spi.h"
#include "sdkconfig.h"

static spi_device_handle_t spi;

spi_device_handle_t my_spi_init_for_imu()
{
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = 4,  // IO4
        .miso_io_num = 7,  // IO7
        .sclk_io_num = 10, // IO10
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

    return spi;
}