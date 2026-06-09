#pragma once
#include "driver/gpio.h"
#include <stdio.h>
#include <stdbool.h>

#define BTN_LEFT GPIO_NUM_9
#define BTN_RIGHT GPIO_NUM_2

void buttons_init(void);
bool button_is_pressed(uint8_t pin);
void buttons_debug(void);