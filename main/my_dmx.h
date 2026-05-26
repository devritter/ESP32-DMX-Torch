#pragma once
#include "esp_err.h"

esp_err_t my_dmx_init();
esp_err_t my_dmx_send_frame(uint8_t* channels, uint16_t num_channels);