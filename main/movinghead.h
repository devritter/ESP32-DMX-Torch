#pragma once

// DMX Werte für den MH-X25 (12-Kanal-Modus, 1-basiert)
#define MH_CH_PAN 1
#define MH_CH_TILT 2
#define MH_CH_PAN_16 3
#define MH_CH_TILT_16 4

#define MH_CH_SPEED 5
#define MH_CH_COLOR 6
#define MH_CH_SHUTTER 7
#define MH_CH_DIMMER 8
#define MH_CH_GOBO 9
#define MH_CH_GOBO_ROT 10
#define MH_CH_SETTINGS 11
#define MH_CH_PROGRAMS 12

typedef struct
{
    uint16_t start_address; // DMX-Startadresse (z.B. 1)

    uint8_t pan_coarse;  // Kanal 1: Pan Grobeinstellung (0-255)
    uint8_t pan_fine;    // Kanal 3: Pan Feineinstellung (0-255)
    uint8_t tilt_coarse; // Kanal 2: Tilt Grobeinstellung (0-255)
    uint8_t tilt_fine;   // Kanal 4: Tilt Feineinstellung (0-255)

    uint8_t speed;    // Kanal 5: Geschwindigkeit Pan/Tilt (0=schnell, 255=langsam)
    uint8_t color;    // Kanal 6: Farbrad (0-255)
    uint8_t shutter;  // Kanal 7: Shutter / Strobe (0-255, offen meist bei 255)
    uint8_t dimmer;   // Kanal 8: Helligkeit (0-255)
    uint8_t gobo;     // Kanal 9: Goborad (0-255)
    uint8_t gobo_rot; // Kanal 10: Gobo-Rotation (0-255)
    uint8_t settings; // Kanal 11: Prisma (0-255)
    uint8_t special;  // Kanal 12: Funktionen / Reset / Interne Programme (0-255)
} mh_x25_t;

void mh_x25_demo(uint16_t start_address);
void mh_x25_fill_buffer(const mh_x25_t *movinghead, uint8_t *dmx_buffer);