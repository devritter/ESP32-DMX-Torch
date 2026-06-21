#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "buttons.h"
#include "my_led_matrix.h"
#include "my_spi.h"
#include "display.h"
#include "dmx.h"
#include "filter.h"
#include "imu.h"
#include "movinghead.h"
#include "utils.h"
#include "screen_main.h"
#include "teleplot.h"

#define RAD_TO_DEG 57.295779513f // 180 / PI
#define DMX_COLOR_INDEX (6 - 1)
#define LEN(x) (sizeof(x) / sizeof(x[0]))
#define MH_PAN_OFFSET_DEG -40 // so it works better on the desk
#define MH_TILT_OFFSET_DEG 90 // so it works better on the desk

static u8g2_t *display;
static uint8_t dmx_buffer[512 + 1] = {0};

static imu_xyz_t acc_data_filtered = {};
// static imu_xyz_t gyro_data_filtered = {};
static imu_xyz_t acc_data_raw = {};
// static imu_xyz_t gyro_data_raw = {};
static mh_x25_t movinghead = {
    .start_address = 1,
    .pan_coarse = 127,
    .tilt_coarse = 127,
    .shutter = 255,
    .color = 15,
    .gobo = 0, // 0 = full beam, 60 = small dot
    .speed = 127,
    .dimmer = 25};

static void update_movinghead(mh_x25_t *movinghead, float pitch, float roll);

void app_main(void)
{
    buttons_init();
    my_led_matrix_setup(CONFIG_BLINK_GPIO);
    my_led_matrix_set_rgb(0, 0, 16);

    spi_device_handle_t imu_spi_device = my_spi_init_for_imu();
    imu_init(imu_spi_device);
    ESP_ERROR_CHECK(imu_test_connection());

    dmx_init();

    display = display_init();

    // mh_x25_demo(1);
    // movinghead_test();

    while (1)
    {
        teleplot_init_once_if_button_pressed();

        // ### Read data ###

        // ESP_ERROR_CHECK(imu_read_gyro_data(&gyro_data_raw));
        // filter_imu(&gyro_data_filtered, &gyro_data_raw);
        // teleplot_send_imu("Gyro_raw_", &gyro_data_raw);
        // teleplot_send_imu("Gyro_filtered_", &gyro_data_filtered);

        ESP_ERROR_CHECK(imu_read_acc_data(&acc_data_raw));
        filter_imu(&acc_data_filtered, &acc_data_raw);
        teleplot_send_imu("Acc_raw_", &acc_data_raw);
        teleplot_send_imu("Acc_filtered_", &acc_data_filtered);

        // ### Calculate angles ###

        float pan_rad = atan2(-acc_data_filtered.x, sqrt(acc_data_filtered.y * acc_data_filtered.y + acc_data_filtered.z * acc_data_filtered.z));
        float tilt_rad = atan2(acc_data_filtered.y, acc_data_filtered.z);
        float pan_deg = pan_rad * RAD_TO_DEG;
        float tilt_deg = tilt_rad * RAD_TO_DEG;

        // ### Visualize data and send it to the MovingHead ###

        my_led_matrix_show_pan_tilt(pan_deg, tilt_deg);
        update_movinghead(&movinghead, pan_deg + MH_PAN_OFFSET_DEG, tilt_deg + MH_TILT_OFFSET_DEG);
        screen_main_render(display, &acc_data_filtered, pan_deg, tilt_deg);
        teleplot_send_cube(tilt_rad, pan_rad);

        // just some delay because faster processing isn't needed and we have some reserve for the future
        sleep_ms(10);
    }
}

// 
static void update_movinghead(mh_x25_t *movinghead, float pan, float tilt)
{
    const uint8_t center_value = 127;
    movinghead->pan_coarse = center_value + pan;
    movinghead->tilt_coarse = center_value + tilt;
    mh_x25_fill_buffer(movinghead, dmx_buffer);
    dmx_send(dmx_buffer, 20); // 20 channels are sufficcient for now
}