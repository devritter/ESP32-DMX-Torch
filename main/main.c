#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "my_spi.h"
#include <math.h>
#define RAD_TO_DEG 57.295779513f // 180 / PI

static const char *TAG = "example";
static led_strip_handle_t led_strip;

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    spi_device_handle_t spi_device = my_spi_setup();

    uint8_t who_am_i = my_spi_read_register(0x75); // 0x75=WHO_AM_I
    printf("WHO_AM_I = %d (expected: %d)\n", who_am_i, 0x47);

    ESP_ERROR_CHECK(my_spi_write_register(0x4E, 0b00001111)); // 0x4E = Power Management
    // 200us no writes, 45ms for the gyro
    vTaskDelay(100 / portTICK_PERIOD_MS);

    float acc_scale_factor = 2048.0f;

    while (1)
    {
        uint8_t temp_high = my_spi_read_register(0x1D);
        uint8_t temp_low = my_spi_read_register(0x1E);
        int16_t temp_raw = temp_high << 8 | temp_low;
        float temp_centidegree = ((float)temp_raw / 132.48f) + 25.0f;
        printf("Gyro internal temperature: %f (internal: %d %d)\n", temp_centidegree, temp_low, temp_high);

        // todo SPI burst
        uint8_t gyro_x_1 = my_spi_read_register(0x25);
        uint8_t gyro_x_0 = my_spi_read_register(0x26);
        uint8_t gyro_y_1 = my_spi_read_register(0x27);
        uint8_t gyro_y_0 = my_spi_read_register(0x28);
        uint8_t gyro_z_1 = my_spi_read_register(0x29);
        uint8_t gyro_z_0 = my_spi_read_register(0x2A);

        uint8_t acc_x_1 = my_spi_read_register(0x1F);
        uint8_t acc_x_0 = my_spi_read_register(0x20);
        uint8_t acc_y_1 = my_spi_read_register(0x21);
        uint8_t acc_y_0 = my_spi_read_register(0x22);
        uint8_t acc_z_1 = my_spi_read_register(0x23);
        uint8_t acc_z_0 = my_spi_read_register(0x24);

        float acc_raw_x = (int16_t)(acc_x_1 << 8 | acc_x_0);
        float acc_raw_y = (int16_t)(acc_y_1 << 8 | acc_y_0);
        float acc_raw_z = (int16_t)(acc_z_1 << 8 | acc_z_0);

        float acc_x = (float)acc_raw_x / acc_scale_factor;
        float acc_y = (float)acc_raw_y / acc_scale_factor;
        float acc_z = (float)acc_raw_z / acc_scale_factor;
        // printf("Acc: %d %d %d\n", acc_x, acc_y, acc_z);

        // roll = x-axis
        // pitch = y-axis
        float roll = atan2(acc_y, acc_z) * RAD_TO_DEG;
        float pitch = atan2(-acc_x, sqrt(acc_y * acc_y + acc_z * acc_z)) * RAD_TO_DEG;
        // printf("Roll: %f      Pitch: %f\n", roll, pitch);

        uint8_t matrix_x = 2;
        uint8_t matrix_y = 2;

        if (false)
            ;
        else if (pitch < -20)
            matrix_x -= 2;
        else if (pitch < -10)
            matrix_x -= 1;
        else if (pitch > +20)
            matrix_x += 2;
        else if (pitch > +10)
            matrix_x += 1;

        if (false)
            ;
        else if (roll < -20)
            matrix_y -= 2;
        else if (roll < -10)
            matrix_y -= 1;
        else if (roll > +20)
            matrix_y += 2;
        else if (roll > +10)
            matrix_y += 1;

        led_strip_clear(led_strip);
        // printf("Pixel: X=%d Y=%d\n", matrix_x, matrix_y);
        my_led_matrix_set_pixel_xy(matrix_x, matrix_y);
        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
