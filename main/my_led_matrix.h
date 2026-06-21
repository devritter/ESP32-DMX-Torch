#pragma once
#include <stdio.h>
#include "led_strip.h"

led_strip_handle_t my_led_matrix_setup(uint8_t gpio_pin);
esp_err_t my_led_matrix_set_rgb(uint8_t r, uint8_t g, uint8_t b);
esp_err_t my_led_matrix_show_number(uint8_t number);
esp_err_t my_led_matrix_set_pixel_xy(uint8_t x, uint8_t y);
esp_err_t my_led_matrix_clear_pixel_xy(uint8_t x, uint8_t y);
esp_err_t my_led_matrix_show_pan_tilt(float pan, float tilt);