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

#include "util/spi_master_lobo.h"
#include "util/tftspi.h"
#include "util/tft.h"

#include "m5power.h"
#include "m5button.h"
#include "m5display.h"

#define I2C_SDA_GPIO (gpio_num_t) 21    /*!< SDA pin of I²C bus */
#define I2C_SCL_GPIO (gpio_num_t) 22    /*!< SCL pin of I²C bus */

extern esp_event_loop_handle_t m5_event_loop;   /*!< Event loop for M5 device-specific events */

/**
 * @brief   Initialize M5StickC
 * 
 *          Initializes power management, display and buttons.
 * 
 * @return  ESP_OK success
 *          ESP_FAIL errors found
 */
esp_err_t m5_init();

#ifdef __cplusplus
}
#endif

#endif // _M5STICKC_H_
