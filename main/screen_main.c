#include "imu.h"
#include "screen_main.h"

static char display_line_buffer[128 + 1] = {};

void screen_main_render(u8g2_t *display, imu_xyz_t *acc_data)
{
    // u8g2_ClearBuffer(display);

    u8g2_FirstPage(display);
    do
    {
        u8g2_DrawStr(display, 0, 8, "ESP-DMX-Torch");
        u8g2_DrawStr(display, 0, 16, "Acc:");
        // u8g2_DrawStr(display, 64, 16, "Gyro:");
        snprintf(display_line_buffer, sizeof(display_line_buffer), "X: %6.2f", acc_data->x);
        u8g2_DrawStr(display, 0, 32, display_line_buffer);
        snprintf(display_line_buffer, sizeof(display_line_buffer), "Y: %6.2f", acc_data->y);
        u8g2_DrawStr(display, 0, 40, display_line_buffer);
        snprintf(display_line_buffer, sizeof(display_line_buffer), "Z: %6.2f", acc_data->z);
        u8g2_DrawStr(display, 0, 48, display_line_buffer);
    } while (u8g2_NextPage(display));

    // u8g2_SendBuffer(display);
}