#include "my_dmx.h"
#include "driver/uart.h"

#define DMX_UART_NUM UART_NUM_1
#define DMX_TX_PIN 21    // 20 oder 21
#define DMX_BUF_SIZE 513 // 1 Start-Byte (0x00) + 512 Kanäle

esp_err_t my_dmx_init()
{
    uart_config_t uart_config = {
        .baud_rate = 250000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    // UART konfigurieren und installieren
    esp_err_t ret = ESP_OK;
    ret = uart_param_config(DMX_UART_NUM, &uart_config);
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = uart_set_pin(DMX_UART_NUM, DMX_TX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK)
    {
        return ret;
    }
    ret = uart_driver_install(DMX_UART_NUM, DMX_BUF_SIZE * 2, 0, 0, NULL, 0);
    return ret;
}

esp_err_t my_dmx_send_frame(uint8_t *channels, uint16_t num_channels)
{
    uint8_t start_byte = 0x00; // Standard für Helligkeitsdaten

    // 1. DMX Break signalisieren (mindestens 88µs Low)
    uart_set_line_inverse(DMX_UART_NUM, UART_SIGNAL_TXD_INV); // TX auf LOW zwingen
    esp_rom_delay_us(100);                                    // Break-Zeit abwarten

    // 2. Mark After Break (MAB) (mindestens 8µs High)
    uart_set_line_inverse(DMX_UART_NUM, 0); // TX wieder normal (HIGH)
    esp_rom_delay_us(12);                   // MAB-Zeit abwarten

    // 3. Start-Byte senden
    uart_write_bytes(DMX_UART_NUM, (const char *)&start_byte, 1);

    // 4. Kanal-Daten senden (z.B. 512 Bytes)
    uart_write_bytes(DMX_UART_NUM, (const char *)channels, num_channels);

    // Optional: Warten bis alles gesendet ist
    uart_wait_tx_done(DMX_UART_NUM, pdMS_TO_TICKS(50));
    return ESP_OK;
}