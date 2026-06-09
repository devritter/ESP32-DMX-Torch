#pragma once
#include "display.h"

void screen_main_render(u8g2_t *display, imu_xyz_t *acc_data, float pan, float tilt);