#include "filter.h"

void filter_value(float *filtered, float *new_value, float alpha)
{
    *filtered = (alpha * (*new_value)) + (1 - alpha) * (*filtered);
}

void filter_imu(imu_xyz_t *filtered, imu_xyz_t *new_data)
{
    static float alpha = 0.25;
    filter_value(&filtered->x, &new_data->x, alpha);
    filter_value(&filtered->y, &new_data->y, alpha);
    filter_value(&filtered->z, &new_data->z, alpha);
}