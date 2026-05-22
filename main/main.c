#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "my_acc_gyro.h"
#include <math.h>
#define RAD_TO_DEG 57.295779513f // 180 / PI

// static const char *TAG = "example";
static led_strip_handle_t led_strip;

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    spi_device_handle_t spi_device = my_acc_gyro_setup();

    uint8_t who_am_i = my_acc_gyro_read(0x75); // 0x75=WHO_AM_I
    printf("WHO_AM_I = %d (expected: %d)\n", who_am_i, 0x47);

    ESP_ERROR_CHECK(my_acc_gyro_write(0x4E, 0b00001111)); // 0x4E = Power Management
    // 200us no writes, 45ms for the gyro
    vTaskDelay(100 / portTICK_PERIOD_MS);

    float acc_scale_factor = 2048.0f;
    my_acc_data_t acc_data = {};

    while (1)
    {
        // printf("Gyro internal temperature: %f\n", my_acc_gyro_read_temperature());

        // todo SPI burst
        uint8_t gyro_x_1 = my_acc_gyro_read(0x25);
        uint8_t gyro_x_0 = my_acc_gyro_read(0x26);
        uint8_t gyro_y_1 = my_acc_gyro_read(0x27);
        uint8_t gyro_y_0 = my_acc_gyro_read(0x28);
        uint8_t gyro_z_1 = my_acc_gyro_read(0x29);
        uint8_t gyro_z_0 = my_acc_gyro_read(0x2A);

        ESP_ERROR_CHECK(my_acc_read_acc_data(&acc_data));
        float acc_x = acc_data.x;
        float acc_y = acc_data.y;
        float acc_z = acc_data.z;
        printf("Acc: %f %f %f\n", acc_x, acc_y, acc_z);

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
