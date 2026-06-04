#pragma once
#include "u8g2.h"

u8g2_t *display_init(void);
void display_write(uint8_t line, char *text);