/**
 * m5display.h
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5DISPLAY_H_
#define _M5DISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_system.h"
#include "esp_log.h"

#include "m5stickc.h"
#include "m5power.h"

#define M5DISPLAY_TYPE DISP_TYPE_ST7735S  /*!< Display type for display driver */
#define M5DISPLAY_WIDTH 160               /*!< Display width in pixels after rotation */
#define M5DISPLAY_HEIGHT 80               /*!< Display height in pixels after rotation */


extern spi_lobo_device_handle_t m5display_spi;    /*!< SPI device handle */

/**
 * @brief   Initialize display
 * 
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t m5display_init();

/**
 * @brief   Set display backlight level
 * 
 * @param   backlight_level Backlight level from 0 (lowest) to 7 (brightest)
 * 
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t m5display_set_backlight_level(uint8_t backlight_level);

esp_err_t m5display_off();

esp_err_t m5display_on();

#ifdef __cplusplus
}
#endif

#endif // _M5DISPLAY_H_