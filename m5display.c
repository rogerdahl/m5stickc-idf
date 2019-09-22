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

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if(cmd == NULL) {
        ESP_LOGE(TAG, "Error creating I2C link");
        return ESP_FAIL;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x28, true);
    i2c_master_write_byte(cmd, ((backlight_level & 0x0f) << 4), true);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "Error setting backlight level: %s", esp_err_to_name(e));
        return ESP_FAIL;
    } 
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}
