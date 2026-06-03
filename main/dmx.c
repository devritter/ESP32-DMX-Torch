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