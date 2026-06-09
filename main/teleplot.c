#include "teleplot.h"
#include "buttons.h"

static bool is_initialized = false;

void teleplot_init()
{
    printf(">3D|cube:S:cube:P:0:0:0:W:7:H:1:D:5:C:blue\n");
    // printf(">3D|spot:S:cube:P:10:0:0:W:2:H:2:D:2:R:0:0:0:C:yellow\n");
    // printf(">3D|movinghead:S:cube:P:0:10:0:W:10:H:2:D:2:C:red\n");

    is_initialized = true;
}

void teleplot_init_once_if_button_pressed()
{
    if (is_initialized)
    {
        return;
    }

    if (button_is_pressed(BTN_RIGHT))
    {
        teleplot_init();
    }
}

void teleplot_send_imu(char *prefix, imu_xyz_t *data)
{
    if (!is_initialized)
    {
        return;
    }

    printf(">%sx:%f\n", prefix, data->x);
    printf(">%sy:%f\n", prefix, data->y);
    printf(">%sz:%f\n", prefix, data->z);
}

void teleplot_send_cube(float tilt_rad, float pan_rad)
{
    if (!is_initialized)
    {
        return;
    }

    printf(">3D|cube:P:0:0:0:R:%f:%f:\n", tilt_rad, pan_rad);
    // float spot_x = cosf(tilt_rad) * cosf(pan_rad);
    // float spot_y = cosf(tilt_rad) * sinf(pan_rad);
    // float spot_z = sinf(tilt_rad);
    // printf(">3D|spot:P:%f:%f:%f\n", spot_x, spot_y, spot_z);
}