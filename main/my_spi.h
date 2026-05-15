#pragma once
#include "esp_err.h"
#include <driver/spi_master.h>

spi_device_handle_t my_spi_setup();
uint8_t my_spi_read_register(uint8_t addr);