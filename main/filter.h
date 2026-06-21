#pragma once
#include "imu.h"

void filter_value(float *filtered, float *new_value, float alpha);
void filter_imu(imu_xyz_t *filtered, imu_xyz_t *new_data);