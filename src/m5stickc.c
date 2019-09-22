/**
 * m5stickc.c - ESP-IDF component to work with M5StickC
 * 
 * Main code file.
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#include "m5stickc.h"

static const char *TAG = "M5STICKC";

spi_lobo_device_handle_t spi;

esp_err_t m5stickc_init() {

    // Init I²C
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_GPIO;
    conf.scl_io_num = I2C_SCL_GPIO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = 100000;
    ESP_LOGI(TAG, "Setting up I2C");
    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error setting up I2C: %s", esp_err_to_name(err));
        return err;
    }
    
    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if(err != ESP_OK) {
        ESP_LOGE(TAG, "Error during I2C driver installation: %s", esp_err_to_name(err));
        return err;
    }

    // Init power management
    m5stickc_init_power();

    // Init IMU

    // Init display
    m5stickc_init_display();

    ESP_LOGI(TAG, "Init done");
    return ESP_OK;
}

esp_err_t m5stickc_init_power() {
    i2c_cmd_handle_t cmd;

    // OLED_VPP enable
    // 0x10 EXTEN & DC-DC2 output control
    // BIT2: EXTEN Switch control. 0 shut down. 1 turn on.
    // BIT0: DC-DC2 Switch control. 0 shut down. 1 turn on.
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x10, true);
    i2c_master_write_byte(cmd, BIT2 | BIT0, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error OLED_VPP enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable DC-DC1, OLED_VDD, 5B V_EXT
    // 0x12 Power supply output control
    // BIT6: EXTEN switch control
    // BIT4: DC-DC2 switch control
    // BIT3: LDO3 switch control
    // BIT2: LDO2 switch control
    // BIT1: DC-DC3 switch control
    // BIT0: DC-DC1 switch control
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x12, true);
    i2c_master_write_byte(cmd, (BIT6 | BIT3 | BIT2 | BIT0), true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error DC-DC1, OLED_VDD, 5B V_EXT enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable LDO2 & LDO3, LED & TFT 3.3V
    // 0x28 LDO2/3 Output Voltage Setting
    // BIT7-4: 1.8-3.3V, 100mV/step
    // BIT3-0: 1.8-3.3V, 100mV/step
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x28, true);
    i2c_master_write_byte(cmd, 0xff, true); // This was 0xff. Just trying things.
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error LDO2 & LDO3, LED & TFT enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable USB thru mode
    // 0x30 VBUS-IPSOUT path management
    // BIT7: 0 N_VBUSEN pin selection. 1 VBUS-IPSOUT input selection regardles of N_VBUSEN status.
    // BIT6: VBUS Vhold limiting control. 0 no limit. 1 limit.
    // BIT5-3: Vhold. 000 4.0V, 001 4.1V, 010 4.2V, 011 4.3V, 100 4.4V, 101 4.5V, 110 4.6V, 111 4.7V.
    // BIT1: VBUS limiting control enable. 0 shutdown. 1 enable.
    // BIT0: VBUS limit control current. 0 500mA, 1 100mA.
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x30, true);
    i2c_master_write_byte(cmd, 0x00, true); // 
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error LDO2 & LDO3, LED & TFT enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable 3.0V ??? What is the Voff function????
    // 0x31 Voff voltage setting
    // BIT3: Sleep mode PWRON press wakeup enable settings. 0 short press to wake up  ?????
    // BIT2-0: Voff setup. 000 2.6V, 001 2.7V, 010 2.8V, 011 2.9V, 100 3.0V, 101 3.1V, 110 3.2V, 111 3.3V
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x31, true);
    i2c_master_write_byte(cmd, 0x00, true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error 3.0V enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable Charging, 100mA, 4.2V, End at 0.9
    // 0x33 Charging Control 1
    // BIT7: Enable control bit, outer and inner channel.
    // BIT6-5: Target voltage. 00 4.1V, 01 4.15V, 10 4.2V, 11 4.36V
    // BIT4: Current setting at the end of charge. 0 current is less than 10% whem the end of charging set value. 1 15%.
    // BIT 3-0: Current setting internal passage.   0000 100mA, 0001 190mA, 0010 280mA, 0011 360mA
    //                                              0100 450mA, 0101 550mA, 0110 630mA, 0111 700mA
    //                                              1000 780mA, 1001 880mA, 1010 960mA, 1011 1000mA
    //                                              1100 1080mA, 1101 1160mA, 1110 1240mA, 1111 1320mA
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x33, true);
    i2c_master_write_byte(cmd, 0xc0, true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error Charging enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable PEK
    // 0x36 PEK key parameters
    // BIT7-6: boot time settings. 00 128ms, 01 512ms, 10 1s, 11 2s.
    // BIT5-4: long time setting key. 00 1s, 01 1.5s, 10 2s, 11 2.5s.
    // BIT3: Automatic shutdown function ???
    //5 BIT2: PWROK signal delay after power-up complete. 0 32ms, 1 64ms.
    // BIT1-0: Long set off. 00 4s, 01 6s, 10 8s, 11 10s.
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x36, true);
    i2c_master_write_byte(cmd, (BIT6 | BIT4 | BIT3 | BIT2), true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error PEK enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable ADCs
    // 0x82 ADC Enable 1. 0 shut down. 1 turn on.
    // BIT7: Battery voltage ADC enable
    // BIT6: Battery current ADC enable
    // BIT5: ACIN voltage ADC enable
    // BIT4: ACIN electric current ADC enable
    // BIT3: VBUS voltage ADC enable
    // BIT2: VBUS electric current ADC enable
    // BIT1: APS voltage ADC enable
    // BIT0: TS pin ADC enable function
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x82, true);
    i2c_master_write_byte(cmd, 0xff, true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error ADCs enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable GPIO0
    // 0x90 GPIO0 feature
    // BIT2-0: 000 NMOS Open-drain output, 001 Universal input function, 010 Low noise LDO, 011 Retention.
    //         100 ADC entry, 101 output low, 11x floating
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x90, true);
    i2c_master_write_byte(cmd, 0x02, true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error GPIO0 enable");
    } 
    i2c_cmd_link_delete(cmd);

    // Enable Coulomb counter
    // 0xB8 Coulomb gauge control
    // BIT7: Switching control coulomb meter
    // BIT6: meter pause control. 1 pause metering. 0 resume.
    // BIT5: clear measurement
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0xB8, true);
    i2c_master_write_byte(cmd, 0x80, true);
    i2c_master_stop(cmd);
    err = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error Coulomb counter enable");
    } 
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}

esp_err_t m5stickc_init_display() {
    tft_disp_type = DEFAULT_DISP_TYPE;
    _width = DEFAULT_TFT_DISPLAY_WIDTH;  // smaller dimension
    _height = DEFAULT_TFT_DISPLAY_HEIGHT; // larger dimension
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

    esp_err_t err = spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &spi);
    if(err != ESP_OK) {
        return err;
    }
    disp_spi = spi;

    // ==== Test select/deselect ====
    spi_lobo_device_select(spi, 1);
    spi_lobo_device_deselect(spi);

    gpio_set_level((gpio_num_t) PIN_NUM_RST, 1);
    vTaskDelay(5/portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t) PIN_NUM_RST, 0);
    vTaskDelay(20/portTICK_PERIOD_MS);
    gpio_set_level((gpio_num_t) PIN_NUM_RST, 1);

    TFT_display_init();
    TFT_invertDisplay(INVERT_ON);
    //disp_spi_transfer_cmd(TFT_INVONN);

    // ==== Set SPI clock used for display operations ====
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
    
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

esp_err_t m5stickc_set_display_brightness(uint8_t brightness) {
    esp_err_t e;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if(cmd == NULL) {
        ESP_LOGE(TAG, "I2C link creation failed");
        return ESP_FAIL;
    }

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP192_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, 0x28, true);
    i2c_master_write_byte(cmd, ((brightness & 0x0f) << 4), true);
    i2c_master_stop(cmd);
    e = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10/portTICK_PERIOD_MS);
    if (e != ESP_OK) {
        ESP_LOGE(TAG, "Error setting brightness: %s", esp_err_to_name(e));
        return e;
    } 
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}
