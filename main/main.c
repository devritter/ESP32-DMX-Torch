#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "my_spi.h"

static const char *TAG = "example";
static led_strip_handle_t led_strip;

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    spi_device_handle_t spi_device = my_spi_setup();

    vTaskDelay(500 / portTICK_PERIOD_MS);

    while (1)
    {
        for (uint8_t i = 0; i < 50; i++)
        {
            my_led_matrix_show_number(i);

            uint8_t data_from_spi = my_spi_read_register(0x75); // 0x75=WHO_AM_I
            printf("WHO_AM_I = %d (expected: %d)\n", data_from_spi, 0x47);

            vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        }
    }
}
