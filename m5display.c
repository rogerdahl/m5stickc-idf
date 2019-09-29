/**
 * m5display.c
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#include "m5display.h"

static const char * TAG = "m5display";

spi_lobo_device_handle_t m5display_spi;

esp_err_t m5display_init() {
    esp_err_t e;

    tft_disp_type = M5DISPLAY_TYPE;
    _width = 80;  // smaller dimension
    _height = 160; // larger dimension
    max_rdclock = 8000000;

    TFT_PinsInit();

    spi_lobo_bus_config_t buscfg = {};
    buscfg.miso_io_num = PIN_NUM_MISO;				// set SPI MISO pin
    buscfg.mosi_io_num = PIN_NUM_MOSI;				// set SPI MOSI pin
    buscfg.sclk_io_num = PIN_NUM_CLK;				// set SPI CLK pin
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 6*1024;

    spi_lobo_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz=8000000;                // Initial clock out at 8 MHz
    devcfg.mode=0;                                // SPI mode 0
    devcfg.spics_io_num=-1;                       // we will use external CS pin
    devcfg.spics_ext_io_num=PIN_NUM_CS;           // external CS pin
    devcfg.flags=LB_SPI_DEVICE_HALFDUPLEX;        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi

    e = spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &m5display_spi);
    if(e != ESP_OK) {
        ESP_LOGE(TAG, "Error adding display to SPI bus: %s", esp_err_to_name(e));
        return ESP_FAIL;
    }
    disp_spi = m5display_spi;

    // ==== Test select/deselect ====
    spi_lobo_device_select(m5display_spi, 1);
    spi_lobo_device_deselect(m5display_spi);

    // Pin reset
    gpio_set_level((gpio_num_t) PIN_NUM_RST, 1);
    vTaskDelay(5/portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t) PIN_NUM_RST, 0);
    vTaskDelay(20/portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t) PIN_NUM_RST, 1);

    TFT_display_init();
    TFT_invertDisplay(INVERT_ON);

    // ==== Set SPI clock used for display operations ====
    spi_lobo_set_speed(m5display_spi, DEFAULT_SPI_CLOCK);
    
    font_rotate = 0;
    text_wrap = 0;
    font_transparent = 0;
    font_forceFixed = 0;
    gray_scale = 0;
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(LANDSCAPE);
    TFT_setFont(DEFAULT_FONT, NULL);
    TFT_resetclipwin();

    return ESP_OK;
}

esp_err_t m5display_set_backlight_level(uint8_t backlight_level) {
    esp_err_t e;
    uint8_t register_content;

    e = m5power_register_read(LDO2_LDO3_VOLTAGE_SETTING_REG, &register_content);
    if(e != ESP_OK) {
        ESP_LOGE(TAG, "Error setting reading register to set backlight level: %s", esp_err_to_name(e));
        return ESP_FAIL;
    }

    register_content &= 0x0F; // Clear 4 msb
    register_content |= (0x80 | (backlight_level << 4)); // Set value. Values less than 0x80 are completely off.

    e = m5power_register_write(LDO2_LDO3_VOLTAGE_SETTING_REG, register_content);
    if(e != ESP_OK) {
        ESP_LOGE(TAG, "Error writing register to set backlight level: %s", esp_err_to_name(e));
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t m5display_off()
{
    esp_err_t e;

    e = m5power_register_unset_bits(DCDC1_DCDC3_LDO2_LDO3_SWITCH_CONTROL_REG, BIT2);
    if(e == ESP_OK) {
        ESP_LOGD(TAG, "Turned off");
    } else {
        ESP_LOGD(TAG, "Error turning off: %s", esp_err_to_name(e));
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t m5display_on()
{
    esp_err_t e;

    e = m5power_register_set_bits(DCDC1_DCDC3_LDO2_LDO3_SWITCH_CONTROL_REG, BIT2);
    if(e == ESP_OK) {
        ESP_LOGD(TAG, "Turned on");
    } else {
        ESP_LOGD(TAG, "Error turning on: %s", esp_err_to_name(e));
        return ESP_FAIL;
    }
    
    return ESP_OK;
}