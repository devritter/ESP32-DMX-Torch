#pragma once
#include "imu.h"

void teleplot_init();
void teleplot_init_once_if_button_pressed();
void teleplot_send_imu(char *prefix, imu_xyz_t *data);
void teleplot_send_cube(float tilt_rad, float pan_rad);