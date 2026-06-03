#include "dmx.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define DMX_UART_NUM UART_NUM_1 // Verwende UART1 für DMX
#define DMX_TX_IO GPIO_NUM_21   // Modul-Pin 31 -> SP3485 DI
#define DMX_EN_IO GPIO_NUM_9    // RS485 Sende-Aktivierung
// Falls du den RX-Pin des SP3485 (RO) auch verbunden hast:
#define DMX_RX_IO GPIO_NUM_20 // Modul-Pin 30 -> SP3485 RO
#define DMX_BAUD_RATE 250000
#define DMX_BUF_SIZE 513 // 1 Startbyte (0x00) + 512 Kanäle

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

void dmx_send(uint8_t *data, uint16_t num_channels);

void dmx_test(void)
{
    dmx_init();

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
    dmx_buffer[MH_CH_GOBO - 1] = 0;      // Volle Helligkeit
    dmx_buffer[MH_CH_GOBO_ROT - 1] = 0;      // Volle Helligkeit

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

void dmx_init(void)
{
    // 1. RS485 Enable-Pin konfigurieren
    gpio_reset_pin(DMX_EN_IO);
    gpio_set_direction(DMX_EN_IO, GPIO_MODE_OUTPUT);
    gpio_set_level(DMX_EN_IO, 1); // Auf HIGH setzen = Senden aktivieren

    // 2. UART Konfiguration für DMX (250k Baud, 8N2)
    uart_config_t uart_config = {
        .baud_rate = DMX_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(DMX_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(DMX_UART_NUM, DMX_TX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(DMX_UART_NUM, DMX_BUF_SIZE * 2, 0, 0, NULL, 0));
}

void dmx_send(uint8_t *data, uint16_t num_channels)
{
    if (num_channels > 512)
        num_channels = 512;

    uint8_t dmx_packet[513];
    dmx_packet[0] = 0x00; // Start Code für normale DMX-Daten
    memcpy(&dmx_packet[1], data, num_channels);

    // Break-Signal
    // Sendet eine Null mit verlängertem Break (DMX Spezifikation)
    uart_write_bytes_with_break(DMX_UART_NUM, "\0", 1, 20); // 20 Bit-Zeiten Break

    // 2. Eigentliche DMX-Daten senden
    uart_write_bytes(DMX_UART_NUM, (const char *)dmx_packet, num_channels + 1);
    ESP_ERROR_CHECK(uart_wait_tx_done(DMX_UART_NUM, portMAX_DELAY));
}