#include <stdio.h>
#include "my_led_matrix.h"
#include "led_strip.h"

#define LED_OFF 255
#define LED_MAX_INDEX 24
#define LEN(x) (sizeof(x) / sizeof(x[0]))

static void set_pixel_from_array(uint8_t array[], size_t size);

static led_strip_handle_t led_strip;
static uint8_t r = 16;
static uint8_t g = 16;
static uint8_t b = 16;

// 00 01 02 03 04
// 05 06 07 08 09
// 10 11 12 13 14
// 15 16 17 18 19
// 20 21 22 23 24

static uint8_t number_0_leds[] = {2, 3, 4, 9, 14, 19, 24, 23, 22, 17, 12, 7};
static uint8_t number_1_leds[] = {7, 3, 8, 13, 18, 23};
static uint8_t number_2_leds[] = {7, 3, 9, 14, 18, 22, 23, 24};
static uint8_t number_3_leds[] = {2, 3, 9, 13, 19, 23, 22};
static uint8_t number_4_leds[] = {2, 7, 12, 13, 14, 18, 23};
static uint8_t number_5_leds[] = {4, 3, 2, 7, 12, 13, 19, 23, 22};
static uint8_t number_6_leds[] = {4, 3, 2, 7, 12, 17, 22, 23, 24, 19, 14, 13};
static uint8_t number_7_leds[] = {2, 3, 4, 9, 13, 18, 23};
static uint8_t number_8_leds[] = {2, 3, 4, 9, 14, 19, 24, 23, 22, 17, 12, 7, 13};
static uint8_t number_9_leds[] = {13, 12, 7, 2, 3, 4, 9, 14, 19, 24, 23, 22};

led_strip_handle_t my_led_matrix_setup(uint8_t gpio_pin)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_pin,
        .max_leds = LED_MAX_INDEX + 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);

    return led_strip;
}

esp_err_t my_led_matrix_set_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    // todo error handling
    r = red;
    g = green;
    b = blue;
    return ESP_OK;
}

esp_err_t my_led_matrix_show_number(uint8_t number)
{
    if (number > 99)
    {
        return ESP_ERR_INVALID_ARG;
    }

    led_strip_clear(led_strip);

    switch (number % 10)
    {
    case 0:
    {
        set_pixel_from_array(number_0_leds, LEN(number_0_leds));
        break;
    }
    case 1:
    {
        set_pixel_from_array(number_1_leds, LEN(number_1_leds));
        break;
    }
    case 2:
    {
        set_pixel_from_array(number_2_leds, LEN(number_2_leds));
        break;
    }
    case 3:
    {
        set_pixel_from_array(number_3_leds, LEN(number_3_leds));
        break;
    }
    case 4:
    {
        set_pixel_from_array(number_4_leds, LEN(number_4_leds));
        break;
    }
    case 5:
    {
        set_pixel_from_array(number_5_leds, LEN(number_5_leds));
        break;
    }
    case 6:
    {
        set_pixel_from_array(number_6_leds, LEN(number_6_leds));
        break;
    }
    case 7:
    {
        set_pixel_from_array(number_7_leds, LEN(number_7_leds));
        break;
    }
    case 8:
    {
        set_pixel_from_array(number_8_leds, LEN(number_8_leds));
        break;
    }
    case 9:
    {
        set_pixel_from_array(number_9_leds, LEN(number_9_leds));
        break;
    }
    }

    switch (number / 10)
    {
    case 1:
    {
        my_led_matrix_set_pixel_xy(0, 0);
        break;
    }
    case 2:
    {
        my_led_matrix_set_pixel_xy(0, 0);
        my_led_matrix_set_pixel_xy(0, 1);
        break;
    }
    case 3:
    {
        my_led_matrix_set_pixel_xy(0, 0);
        my_led_matrix_set_pixel_xy(0, 1);
        my_led_matrix_set_pixel_xy(0, 2);
        break;
    }
    case 4:
    {
        my_led_matrix_set_pixel_xy(0, 0);
        my_led_matrix_set_pixel_xy(0, 1);
        my_led_matrix_set_pixel_xy(0, 2);
        my_led_matrix_set_pixel_xy(0, 3);
        break;
    }
    case 5:
    {
        my_led_matrix_set_pixel_xy(0, 0);
        my_led_matrix_set_pixel_xy(0, 1);
        my_led_matrix_set_pixel_xy(0, 2);
        my_led_matrix_set_pixel_xy(0, 3);
        my_led_matrix_set_pixel_xy(0, 4);
        break;
    }
    case 6:
    {
        my_led_matrix_set_pixel_xy(0, 1);
        my_led_matrix_set_pixel_xy(0, 2);
        my_led_matrix_set_pixel_xy(0, 3);
        my_led_matrix_set_pixel_xy(0, 4);
        break;
    }
    case 7:
    {
        my_led_matrix_set_pixel_xy(0, 2);
        my_led_matrix_set_pixel_xy(0, 3);
        my_led_matrix_set_pixel_xy(0, 4);
        break;
    }
    case 8:
    {
        my_led_matrix_set_pixel_xy(0, 3);
        my_led_matrix_set_pixel_xy(0, 4);
        break;
    }
    case 9:
    {
        my_led_matrix_set_pixel_xy(0, 4);
        break;
    }
    }

    led_strip_refresh(led_strip);
    return ESP_OK;
}

static void set_pixel_from_array(uint8_t pixels[], size_t pixels_size)
{
    for (int8_t i = 0; i < pixels_size; i++)
    {
        uint8_t led = pixels[i];
        if (led != LED_OFF)
        {
            led_strip_set_pixel(led_strip, led, r, g, b);
        }
    }
}

esp_err_t my_led_matrix_set_pixel_xy(uint8_t x, uint8_t y)
{
    uint8_t final_index = 5 * y + x;
    if (final_index > LED_MAX_INDEX)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return led_strip_set_pixel(led_strip, final_index, r, g, b);
}

esp_err_t my_led_matrix_clear_pixel_xy(uint8_t x, uint8_t y)
{
    uint8_t final_index = 5 * y + x;
    if (final_index > LED_MAX_INDEX)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return led_strip_set_pixel(led_strip, final_index, 0, 0, 0);
}

static uint8_t get_pixel_by_degree(float degree)
{
    uint8_t pixel = 2;

    if (degree < -10)
        pixel--;
    if (degree < -20)
        pixel--;
    if (degree > 10)
        pixel++;
    if (degree > 20)
        pixel++;

    return pixel;
}

esp_err_t my_led_matrix_show_pan_tilt(float pan, float tilt)
{
    uint8_t matrix_x = get_pixel_by_degree(pan);
    uint8_t matrix_y = get_pixel_by_degree(tilt);

    esp_err_t ret = led_strip_clear(led_strip);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = my_led_matrix_set_pixel_xy(matrix_x, matrix_y);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = led_strip_refresh(led_strip);
    return ret;
}