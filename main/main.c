#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"

static const char *TAG = "example";
static led_strip_handle_t led_strip;

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);

    while (1)
    {
        for (uint8_t i = 0; i < 50; i++)
        {
            my_led_matrix_show_number(i);
            vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        }
    }
}
