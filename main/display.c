#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "display.h"
#include "u8g2.h"

/* I2C Configuration */
#define I2C_MASTER_NUM 0    /*!< I2C master port number */
#define I2C_MASTER_SDA_IO 5 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_SCL_IO 6 /*!< GPIO number used for I2C master clock */
#define I2C_FREQ_HZ 400000  /*!< I2C master clock frequency */
#define I2C_TIMEOUT_MS 1000 /*!< I2C master timeout */

/* Display Configuration */
#define I2C_DISPLAY_ADDRESS 0x3C /*!< Display I2C address */

static const char *TAG = "Display";
static u8g2_t u8g2 = {};

static i2c_master_bus_handle_t i2c_bus_handle = NULL;     /*!< I2C master bus handle */
static i2c_master_dev_handle_t display_dev_handle = NULL; /*!< Display device handle */

static uint8_t u8x8_byte_i2c_cb(u8x8_t *u8x8, uint8_t msg,
                                uint8_t arg_int, void *arg_ptr);

static uint8_t u8x8_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg,
                                  uint8_t arg_int, void *arg_ptr);

u8g2_t *display_init()
{
    ESP_LOGI(TAG, "U8G2 display init");
    ESP_LOGI(TAG, "I2C Configuration: SDA=GPIO%d, SCL=GPIO%d, Freq=%dHz, Timeout=%dms",
             I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO, I2C_FREQ_HZ, I2C_TIMEOUT_MS);
    ESP_LOGI(TAG, "Display Configuration: Address=0x%02X",
             I2C_DISPLAY_ADDRESS);

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0, /* 0 = synchronous mode */
        .flags.enable_internal_pullup = true,
    };

    /* Create I2C master bus */
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

    /*
     * Initialize U8G2 for your display type.
     * Change the u8g2_Setup_xxx function below to match your display controller,
     * e.g. SSD1306, SH1106, SSD1327, etc.
     * See: https://github.com/olikraus/u8g2/wiki/u8g2setupc
     */
    u8g2_Setup_ssd1306_i2c_128x64_noname_2(
        &u8g2, U8G2_R0,
        u8x8_byte_i2c_cb,  /* I2C communication callback */
        u8x8_gpio_delay_cb /* GPIO and delay callback */
    );

    /* Initialize display hardware */
    ESP_LOGI(TAG, "Initializing display...");
    u8g2_InitDisplay(&u8g2);
    ESP_LOGI(TAG, "Setting power mode...");
    u8g2_SetPowerSave(&u8g2, 0); /* Wake up display */
    ESP_LOGI(TAG, "Display initialization completed");

    u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);

    return &u8g2;
}

void display_write(uint8_t line, char *text)
{
    uint16_t y = (line + 1) * 8;

    u8g2_ClearBuffer(&u8g2);

    /* Small font */
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
    u8g2_DrawStr(&u8g2, 0, y, text);
    // u8g2_DrawStr(&u8g2, 10, 10, "10-10");
    // u8g2_DrawStr(&u8g2, 50, 10, "50-10");
    // u8g2_DrawStr(&u8g2, 10, 50, "10-50");
    // u8g2_DrawStr(&u8g2, 50, 50, "50-50");

    /* Small font */
    // u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
    // u8g2_DrawStr(&u8g2, 0, 44, "Multiple fonts");
    // u8g2_DrawStr(&u8g2, 0, 54, "Text demo");

    u8g2_SendBuffer(&u8g2);
    // vTaskDelay(pdMS_TO_TICKS(3000)); /* delay for showing static display */
}

/**
 * @brief U8X8 I2C communication callback function
 *
 * This function handles all I2C communication between U8G2 library and display controller.
 * Please refer to https://github.com/olikraus/u8g2/wiki/Porting-to-new-MCU-platform for more details.
 *
 * @param[in] u8x8 Pointer to u8x8 structure (not used)
 * @param[in] msg Message type from U8G2 library
 * @param[in] arg_int Integer argument (varies by message type)
 * @param[in] arg_ptr Pointer argument (varies by message type)
 *
 * @return
 *     - 1: Success
 *     - 0: Failure
 */
static uint8_t u8x8_byte_i2c_cb(u8x8_t *u8x8, uint8_t msg,
                                uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[132]; /*!< Enhanced buffer: control byte + 128 data bytes + margin */
    static uint8_t buf_idx;     /*!< Current buffer index */

    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
        /* Add display device to the I2C bus */
        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_DISPLAY_ADDRESS,
            .scl_speed_hz = I2C_FREQ_HZ,
            .scl_wait_us = 0, /* Use default value */
            .flags.disable_ack_check = false,
        };

        esp_err_t ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &display_dev_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "I2C master driver initialized failed");
            return 0;
        }

        ESP_LOGI(TAG, "I2C master driver initialized successfully");
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        /* Start transfer, reset buffer */
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_SET_DC:
        /* DC (Data/Command) control - handled by SSD1306 protocol */
        break;

    case U8X8_MSG_BYTE_SEND:
        /* Add data bytes to buffer */
        for (size_t i = 0; i < arg_int; ++i)
        {
            buffer[buf_idx++] = *((uint8_t *)arg_ptr + i);
        }
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        /* Transmit data using new I2C driver */
        if (buf_idx > 0 && display_dev_handle != NULL)
        {
            esp_err_t ret = i2c_master_transmit(display_dev_handle, buffer, buf_idx, I2C_TIMEOUT_MS);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "I2C master transmission failed");
                return 0;
            }

            /* Debug output: show transmitted data */
            ESP_LOGD(TAG, "Sent %d bytes to 0x%02X: control_byte=0x%02X",
                     buf_idx, I2C_DISPLAY_ADDRESS, buffer[0]);
        }
        break;

    default:
        return 0;
    }
    return 1;
}

/**
 * @brief U8X8 GPIO control and delay callback function for ESP32
 *
 * This function handles GPIO operations and timing delays required by U8G2 library.
 * Please refer to https://github.com/olikraus/u8g2/wiki/Porting-to-new-MCU-platform for more details.
 *
 * @param[in] u8x8 Pointer to u8x8 structure (not used)
 * @param[in] msg Message type from U8G2 library
 * @param[in] arg_int Integer argument (varies by message type)
 * @param[in] arg_ptr Pointer argument (not used)
 *
 * @return
 *     - 1: Success
 *     - 0: Failure
 */
static uint8_t u8x8_gpio_delay_cb(u8x8_t *u8x8, uint8_t msg,
                                  uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        ESP_LOGI(TAG, "GPIO and delay initialization completed");
        break;

    case U8X8_MSG_DELAY_MILLI:
        /* Millisecond delay */
        vTaskDelay(pdMS_TO_TICKS(arg_int));
        break;

    case U8X8_MSG_DELAY_10MICRO:
        /* 10 microsecond delay */
        esp_rom_delay_us(arg_int * 10);
        break;

    case U8X8_MSG_DELAY_100NANO:
        /* 100 nanosecond delay - use minimal delay on ESP32 */
        __asm__ __volatile__("nop");
        break;

    case U8X8_MSG_DELAY_I2C:
        /* I2C timing delay: 5us for 100KHz, 1.25us for 400KHz */
        esp_rom_delay_us(5 / arg_int);
        break;

    case U8X8_MSG_GPIO_RESET:
        /* GPIO reset control (optional for most display controllers) */
        break;

    default:
        /* Other GPIO messages not handled */
        return 0;
    }
    return 1;
}
