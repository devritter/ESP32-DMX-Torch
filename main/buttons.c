#include "buttons.h"
#include "driver/gpio.h"

void buttons_init(void)
{
    // GPIO9 (and therefore BTN_LEFT) not usable on this board because it's also needed for RS485_EN
    // gpio_config_t gpioConfigIn1 = {
    //     .pin_bit_mask = (1 << BTN_LEFT),
    //     .mode = GPIO_MODE_INPUT,
    //     .pull_up_en = true,
    //     .pull_down_en = false,
    //     .intr_type = GPIO_INTR_DISABLE};
    // ESP_ERROR_CHECK(gpio_config(&gpioConfigIn1));
    gpio_config_t gpioConfigIn2 = {
        .pin_bit_mask = (1 << BTN_RIGHT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = true,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE};
    ESP_ERROR_CHECK(gpio_config(&gpioConfigIn2));
}

bool button_is_pressed(uint8_t pin)
{
    return !gpio_get_level(pin);
}

void buttons_debug()
{
    bool left = button_is_pressed(BTN_LEFT);
    bool right = button_is_pressed(BTN_RIGHT);
    printf("Button links:%d \trechts:%d\n", left, right);
}