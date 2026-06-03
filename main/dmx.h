#pragma once
#include <stdint.h>

void dmx_init(void);
void dmx_send(uint8_t *data, uint16_t num_channels);