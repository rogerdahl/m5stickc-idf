/**
 * m5stickc.h - ESP-IDF component to work with M5StickC
 * 
 * Include this header file to use the component.
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5STICKC_H_
#define _M5STICKC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/i2c.h"

#include "display/spi_master_lobo.h"
#include "display/tftspi.h"
#include "display/tft.h"

#define M5STICKC_BUTTON_A   GPIO_NUM_37     /* Button A GPIO */
#define M5STICKC_BUTTON_B   GPIO_NUM_39     /* Button B GPIO */
#define M5STICKC_LED        GPIO_NUM_10     /* Red LED GPIO */
#define M5STICKC_IR_TX      GPIO_NUM_9      /* IR GPIO */

#define I2C_SDA_GPIO (gpio_num_t) 21    /* SDA pin of I²C bus */
#define I2C_SCL_GPIO (gpio_num_t) 22    /* SCL pin of I²C bus */
#define AXP192_I2C_ADDR 0x34            /* I²C address of AXP192 chip */

#define DISPLAY_TYPE DISP_TYPE_ST7735   /* Display type for display driver */
#define DISPLAY_WIDTH 160               /* Display width in pixels after rotation */
#define DISPLAY_HEIGHT 80               /* Display height in pixels after rotation */

extern spi_lobo_device_handle_t spi;    /* SPI device handle */

esp_err_t m5stickc_init();
esp_err_t m5stickc_init_power();
esp_err_t m5stickc_init_display();
esp_err_t m5stickc_set_display_brightness(uint8_t brightness);

#ifdef __cplusplus
}
#endif

#endif // _M5STICKC_H_
