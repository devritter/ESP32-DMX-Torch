#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "imu.h"
#include "dmx.h"
#include "movinghead.h"
#include <math.h>
#define RAD_TO_DEG 57.295779513f // 180 / PI
#define DMX_COLOR_INDEX (6 - 1)
#define LEN(x) (sizeof(x) / sizeof(x[0]))

// static const char *TAG = "example";
static led_strip_handle_t led_strip;
static uint8_t dmx_buffer[513] = {0};

uint8_t get_pixel_by_degree(float degree);
void update_pixel_matrix(float pitch, float roll);
void update_movinghead(mh_x25_t *movinghead, float pitch, float roll);

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    my_led_matrix_set_rgb(0, 0, 16);

    imu_init();
    ESP_ERROR_CHECK(imu_test_connection());

    dmx_init();

    // mh_x25_demo(1);
    // movinghead_test();

    my_acc_gyro_xyz_t acc_data = {};
    my_acc_gyro_xyz_t gyro_data = {};
    mh_x25_t movinghead = {
        .start_address = 1,
        .pan_coarse = 127,
        .tilt_coarse = 127,
        .shutter = 255,
        .color = 15,
        .gobo = 60, // 60 = kleiner Punkt
        .speed = 127,
        .dimmer = 25};

    while (1)
    {
        ESP_ERROR_CHECK(imu_read_gyro_data(&gyro_data));
        printf("Gyro: %.0f %.0f %.0f\n", gyro_data.x, gyro_data.y, gyro_data.z);

        ESP_ERROR_CHECK(imu_read_acc_data(&acc_data));
        // printf("Acc: %f %f %f\n", acc_data.x, acc_data.y, acc_data.z);

        // roll = x-axis, quasi die Achse des USB-C-Anschlusses
        // pitch = y-axis, quasi die Achse ESP-Pixelmatrix
        float roll = atan2(acc_data.y, acc_data.z) * RAD_TO_DEG;
        float pitch = atan2(-acc_data.x, sqrt(acc_data.y * acc_data.y + acc_data.z * acc_data.z)) * RAD_TO_DEG;
        printf("Roll: \t%f  \tPitch: \t%f\n", roll, pitch);

        update_pixel_matrix(pitch, roll);
        update_movinghead(&movinghead, pitch - 40, roll + 90);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void update_movinghead(mh_x25_t *movinghead, float pitch, float roll)
{
    movinghead->pan_coarse = 127 + pitch;
    movinghead->tilt_coarse = 127 + roll;
    mh_x25_fill_buffer(movinghead, dmx_buffer);
    dmx_send(dmx_buffer, 20); // 20 channels are sufficcient for now
}

void update_pixel_matrix(float pitch, float roll)
{
    uint8_t matrix_x = get_pixel_by_degree(pitch);
    uint8_t matrix_y = get_pixel_by_degree(roll);

    led_strip_clear(led_strip);
    my_led_matrix_set_pixel_xy(matrix_x, matrix_y);
    led_strip_refresh(led_strip);
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