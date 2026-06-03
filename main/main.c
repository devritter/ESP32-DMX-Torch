#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "my_acc_gyro.h"
#include "dmx.h"
#include <math.h>
#define RAD_TO_DEG 57.295779513f // 180 / PI

// static const char *TAG = "example";
static led_strip_handle_t led_strip;

uint8_t get_pixel_by_degree(float degree);

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    spi_device_handle_t spi_device = my_acc_gyro_setup();

    uint8_t who_am_i = my_acc_gyro_read(0x75); // 0x75=WHO_AM_I
    printf("WHO_AM_I = %d (expected: %d)\n", who_am_i, 0x47);

    ESP_ERROR_CHECK(my_acc_gyro_write(0x4E, 0b00001111)); // 0x4E = Power Management
    // 200us no writes, 45ms for the gyro
    vTaskDelay(100 / portTICK_PERIOD_MS);

    dmx_test();
    // dmx_init();

    float acc_scale_factor = 2048.0f;
    my_acc_gyro_xyz_t acc_data = {};
    my_acc_gyro_xyz_t gyro_data = {};

    while (1)
    {
        // printf("Gyro internal temperature: %f\n", my_acc_gyro_read_temperature());

        // ESP_ERROR_CHECK(my_acc_gyro_read_gyro_data(&gyro_data));
        // printf("Gyro: %.0f %.0f %.0f\n", gyro_data.x, gyro_data.y, gyro_data.z);

        ESP_ERROR_CHECK(my_acc_gyro_read_acc_data(&acc_data));
        // printf("Acc: %f %f %f\n", acc_data.x, acc_data.y, acc_data.z);

        // roll = x-axis
        // pitch = y-axis
        float roll = atan2(acc_data.y, acc_data.z) * RAD_TO_DEG;
        float pitch = atan2(-acc_data.x, sqrt(acc_data.y * acc_data.y + acc_data.z * acc_data.z)) * RAD_TO_DEG;
        // printf("Roll: %f      Pitch: %f\n", roll, pitch);

        uint8_t matrix_x = get_pixel_by_degree(pitch);
        uint8_t matrix_y = get_pixel_by_degree(roll);

        led_strip_clear(led_strip);
        // printf("Pixel: X=%d Y=%d\n", matrix_x, matrix_y);
        my_led_matrix_set_pixel_xy(matrix_x, matrix_y);
        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

uint8_t get_pixel_by_degree(float degree)
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