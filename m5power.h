/**
 * m5power.h
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#ifndef _M5POWER_H_
#define _M5POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_system.h"
#include "esp_log.h"

#include "util/axp192_reg.h"
#include "m5stickc.h"

#define AXP192_I2C_ADDR 0x34

/**
 * @brief   Initialize power configuring AXP192 IC
 * 
 * @return  ESP_OK success
 *          ESP_FAIL failed
 */
esp_err_t m5power_init();

#ifdef __cplusplus
}
#endif

#endif // _M5POWER_H_