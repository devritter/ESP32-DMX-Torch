#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "movinghead.h"
#include "dmx.h"

static uint8_t dmx_buffer[513] = {0};

void step_through(mh_x25_t *mh, uint8_t *var, uint8_t final_value);

void mh_x25_demo(uint16_t start_address)
{
    mh_x25_t mh = {
        .start_address = start_address,
        mh.pan_coarse = 127,
        mh.tilt_coarse = 127,
        .dimmer = 25,
        .shutter = 255};

    printf("Starting MH-X25 Demo...\n");

    // printf("Pan\n");
    // step_through(&mh, &mh.pan_coarse, 127);

    // printf("Tilt\n");
    // step_through(&mh, &mh.tilt_coarse, 127);

    printf("Color (ab 128: Regenbogen+, ab 192: Regenbogen-)\n");
    step_through(&mh, &mh.color, 15);

    // printf("Dimmer (nur bis 100)\n");
    // step_through(&mh, &mh.dimmer, 25);

    // printf("Shutter\n");
    // step_through(&mh, &mh.shutter, 255);

    // printf("Gobo (ab 65: shake, ab 128: Regenbogen+, ab 192: Regenbogen-: \n");
    // step_through(&mh, &mh.gobo, 25);

    // printf("Gobo Rot (0-63: fixe °, 64-147: Rot+, 148-231: Rot-, 232-255: Bouncing\n");
    // step_through(&mh, &mh.gobo_rot, 255);

    printf("End of MH-X25 Demo!\n");
    vTaskDelay(pdMS_TO_TICKS(5000));
}

void step_through(mh_x25_t *mh, uint8_t *var, uint8_t final_value)
{
    for (uint8_t i = 0; i < 255; i += 5)
    {
        *var = i;
        mh_x25_fill_buffer(mh, dmx_buffer);
        dmx_send(dmx_buffer, 20);
        printf("  var=%u\n", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    *var = final_value;
}

void mh_x25_fill_buffer(const mh_x25_t *movinghead, uint8_t *dmx_buffer)
{
    // Array-Index berechnen (DMX-Adresse minus 1)
    uint16_t base = movinghead->start_address - 1;

    dmx_buffer[base + 0] = movinghead->pan_coarse;
    dmx_buffer[base + 1] = movinghead->tilt_coarse;
    dmx_buffer[base + 2] = movinghead->pan_fine;
    dmx_buffer[base + 3] = movinghead->tilt_fine;
    dmx_buffer[base + 4] = movinghead->speed;
    dmx_buffer[base + 5] = movinghead->color;
    dmx_buffer[base + 6] = movinghead->shutter;
    dmx_buffer[base + 7] = movinghead->dimmer;
    dmx_buffer[base + 8] = movinghead->gobo;
    dmx_buffer[base + 9] = movinghead->gobo_rot;
    dmx_buffer[base + 10] = movinghead->settings;
    dmx_buffer[base + 11] = movinghead->special;
}

void mh_x25_test()
{
    mh_x25_t mh = {
        .start_address = 1,
        .pan_coarse = 127,
        .tilt_coarse = 127,
        .dimmer = 25,
        .shutter = 255};
    uint8_t dmx_buffer[512] = {0};

    while (1)
    {
        // Aktuelle Farbe auf den Farbkanal legen
        dmx_buffer[MH_CH_COLOR - 1] += 5;

        // DMX-Paket raussenden
        dmx_send(dmx_buffer, 20); // Es reicht, die ersten 20

        // warten
        vTaskDelay(pdMS_TO_TICKS(2500));
    }
}