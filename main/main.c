#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "my_led_matrix.h"
#include "my_spi.h"
#include "display.h"
#include "dmx.h"
#include "imu.h"
#include "movinghead.h"
#include "utils.h"
#define RAD_TO_DEG 57.295779513f // 180 / PI
#define DMX_COLOR_INDEX (6 - 1)
#define LEN(x) (sizeof(x) / sizeof(x[0]))

// static const char *TAG = "example";
static led_strip_handle_t led_strip;
static u8g2_t *display;
static char display_line_buffer[128 + 1] = {};
static uint8_t dmx_buffer[512 + 1] = {0};

static imu_xyz_t acc_data_filtered = {};
static imu_xyz_t gyro_data_filtered = {};
static imu_xyz_t acc_data_raw = {};
static imu_xyz_t gyro_data_raw = {};

static uint8_t get_pixel_by_degree(float degree);
static void update_pixel_matrix(float pitch, float roll);
static void update_movinghead(mh_x25_t *movinghead, float pitch, float roll);
static void teleplot_init();
static void filter(imu_xyz_t *filtered, imu_xyz_t *new_data);

void app_main(void)
{
    led_strip = my_led_matrix_setup(CONFIG_BLINK_GPIO);
    my_led_matrix_set_rgb(0, 0, 16);

    spi_device_handle_t imu_spi_device = my_spi_init_for_imu();
    imu_init(imu_spi_device);
    ESP_ERROR_CHECK(imu_test_connection());

    dmx_init();

    display = display_init();

    // mh_x25_demo(1);
    // movinghead_test();

    mh_x25_t movinghead = {
        .start_address = 1,
        .pan_coarse = 127,
        .tilt_coarse = 127,
        .shutter = 255,
        .color = 15,
        .gobo = 60, // 60 = kleiner Punkt
        .speed = 127,
        .dimmer = 25};

    // teleplot_init();

    while (1)
    {
        u8g2_ClearBuffer(display);
        display_write(0, "ESP-DMX-Torch");
        u8g2_DrawStr(display, 0, 16, "Acc:");
        u8g2_DrawStr(display, 64, 16, "Gyro:");

        ESP_ERROR_CHECK(imu_read_gyro_data(&gyro_data_raw));
        filter(&gyro_data_filtered, &gyro_data_raw);
        // printf("Gyro: \t%.0f \t%.0f \t%.0f\n", gyro_data.x, gyro_data.y, gyro_data.z);
        // imu_teleplot("Gyro_raw_", &gyro_data_raw);
        // imu_teleplot("Gyro_filtered_", &gyro_data_filtered);

        ESP_ERROR_CHECK(imu_read_acc_data(&acc_data_raw));
        filter(&acc_data_filtered, &acc_data_raw);
        // printf("Acc: \t%f \t%f \t%f\n", acc_data.x, acc_data.y, acc_data.z);
        // imu_teleplot("Acc_raw_", &acc_data_raw);
        // imu_teleplot("Acc_filtered_", &acc_data_filtered);
        snprintf(display_line_buffer, sizeof(display_line_buffer), "X: %6.2f", acc_data_filtered.x);
        u8g2_DrawStr(display, 0, 32, display_line_buffer);
        snprintf(display_line_buffer, sizeof(display_line_buffer), "Y: %6.2f", acc_data_filtered.y);
        u8g2_DrawStr(display, 0, 40, display_line_buffer);
        snprintf(display_line_buffer, sizeof(display_line_buffer), "Z: %6.2f", acc_data_filtered.z);
        u8g2_DrawStr(display, 0, 48, display_line_buffer);

        // roll = x-axis, quasi die Achse des USB-C-Anschlusses
        // pitch = y-axis, quasi die Achse ESP-Pixelmatrix
        float pan_rad = atan2(acc_data_filtered.y, acc_data_filtered.z);
        float tilt_rad = atan2(-acc_data_filtered.x, sqrt(acc_data_filtered.y * acc_data_filtered.y + acc_data_filtered.z * acc_data_filtered.z));
        float pan_deg = pan_rad * RAD_TO_DEG;
        float tilt_deg = tilt_rad * RAD_TO_DEG;
        // printf("Roll: \t%f  \tPitch: \t%f\n", roll, pitch);

        // printf(">traj:%f:%f|xy\n", pitch, roll);
        update_pixel_matrix(tilt_deg, pan_deg);
        update_movinghead(&movinghead, tilt_deg - 40, pan_deg + 90);

        // printf(">3D|cube:P:0:0:0:R:%f:%f:\n", tilt_rad, pan_rad);
        // float spot_x = cosf(tilt_rad) * cosf(pan_rad);
        // float spot_y = cosf(tilt_rad) * sinf(pan_rad);
        // float spot_z = sinf(tilt_rad);
        // printf(">3D|spot:P:%f:%f:%f\n", spot_x, spot_y, spot_z);

        u8g2_SendBuffer(display);
        sleep_ms(100);
    }
}

static void update_movinghead(mh_x25_t *movinghead, float pitch, float roll)
{
    movinghead->pan_coarse = 127 + pitch;
    movinghead->tilt_coarse = 127 + roll;
    mh_x25_fill_buffer(movinghead, dmx_buffer);
    dmx_send(dmx_buffer, 20); // 20 channels are sufficcient for now
}

static void update_pixel_matrix(float pitch, float roll)
{
    uint8_t matrix_x = get_pixel_by_degree(pitch);
    uint8_t matrix_y = get_pixel_by_degree(roll);

    led_strip_clear(led_strip);
    my_led_matrix_set_pixel_xy(matrix_x, matrix_y);
    led_strip_refresh(led_strip);
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

static void teleplot_init()
{
    printf("wait some time for teleplot...\n");

    update_pixel_matrix(-25, -25);
    sleep_ms(1000);
    update_pixel_matrix(-15, -15);
    sleep_ms(1000);
    update_pixel_matrix(0, 0);
    sleep_ms(1000);
    update_pixel_matrix(15, 15);
    sleep_ms(1000);
    update_pixel_matrix(25, 25);
    sleep_ms(1000);

    printf(">3D|cube:S:cube:P:0:0:0:W:7:H:1:D:5:C:blue\n");
    // printf(">3D|spot:S:cube:P:10:0:0:W:2:H:2:D:2:R:0:0:0:C:yellow\n");
    // printf(">3D|movinghead:S:cube:P:0:10:0:W:10:H:2:D:2:C:red\n");
}

static void filter_value(float *filtered, float *new_value, float alpha)
{
    *filtered = (alpha * (*new_value)) + (1 - alpha) * (*filtered);
}

static void filter(imu_xyz_t *filtered, imu_xyz_t *new_data)
{
    static float alpha = 0.25;
    filter_value(&filtered->x, &new_data->x, alpha);
    filter_value(&filtered->y, &new_data->y, alpha);
    filter_value(&filtered->z, &new_data->z, alpha);
}