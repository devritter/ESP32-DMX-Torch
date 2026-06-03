#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "movinghead.h"
#include "dmx.h"

void movinghead_test(void)
{
    uint8_t dmx_buffer[512] = {0};

    // Farben-Array für den MH-X25 Farbrad-Kanal
    uint8_t farben[3] = {7, 17, 27}; // Gelb, Grün, Blau
    uint8_t farb_index = 0;

    // Basis-Setup für den Moving Head (Startadresse 1 vorausgesetzt)
    dmx_buffer[MH_CH_PAN - 1] = 128; // Kopf mittig ausrichten
    dmx_buffer[MH_CH_TILT - 1] = 128;
    dmx_buffer[MH_CH_SPEED - 1] = 128;
    dmx_buffer[MH_CH_SHUTTER - 1] = 225; // 0 = geschlossen, ab 210 = offen
    dmx_buffer[MH_CH_DIMMER - 1] = 25;
    dmx_buffer[MH_CH_GOBO - 1] = 0;     // Volle Helligkeit
    dmx_buffer[MH_CH_GOBO_ROT - 1] = 0; // Volle Helligkeit

    while (1)
    {
        // Aktuelle Farbe auf den Farbkanal legen
        dmx_buffer[MH_CH_COLOR - 1] = farben[farb_index];

        // DMX-Paket raussenden
        dmx_send(dmx_buffer, 8); // Es reicht, die ersten 8 Kanäle zu senden

        // Index für die nächste Farbe weiterschalten (0 -> 1 -> 2 -> 0...)
        farb_index = (farb_index + 1) % 3;

        // warten
        vTaskDelay(pdMS_TO_TICKS(2500));
    }
}