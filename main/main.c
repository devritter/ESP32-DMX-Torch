#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "my_acc_gyro.h"
#include "my_dmx.h"
#include <math.h>
#define RAD_TO_DEG 57.295779513f // 180 / PI
#define DMX_COLOR_INDEX (6 - 1)
#define LEN(x) (sizeof(x) / sizeof(x[0]))

// static const char *TAG = "example";
static led_strip_handle_t led_strip;

uint8_t get_pixel_by_degree(float degree);

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    my_led_matrix_set_rgb(0, 0, 16);
    spi_device_handle_t spi_device = my_acc_gyro_setup();

    // ESP_ERROR_CHECK(my_dmx_init());

    float acc_scale_factor = 2048.0f;
    my_acc_gyro_xyz_t acc_data = {};
    my_acc_gyro_xyz_t gyro_data = {};
    uint8_t dmx_data[] = {127, 127, 0, 0, 25, 0};

    while (1)
    {
        printf("Gyro WHO_AM_I: %u\n", my_acc_gyro_read(0x75));
        printf("Gyro internal temperature: %f\n", my_acc_gyro_read_temperature());

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

        // dmx_data[DMX_COLOR_INDEX] = matrix_x * 20 + matrix_y * 20;
        // my_dmx_send_frame(dmx_data, LEN(dmx_data));

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