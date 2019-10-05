/**
 * m5stickc.c - ESP-IDF component to work with M5
 *
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#include "m5stickc.h"

static const char *TAG = "m5stickc";

esp_event_loop_handle_t m5_event_loop;

esp_err_t m5_init() {
    esp_err_t e;
    uint8_t error_count = 0;

    esp_event_loop_args_t loop_args = {
        .queue_size = 5,
        .task_name = "m5_event_loop",
        .task_priority = 10,
        .task_stack_size = 2048,
        .task_core_id = 0
    };

    e = esp_event_loop_create(&loop_args, &m5_event_loop);
    if(e == ESP_OK) {
        ESP_LOGD(TAG, "Event loop created");
    } else {
        ESP_LOGE(TAG, "Error creating event loop: %s", esp_err_to_name(e));
        ++error_count;
    }

    // Init I²C
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA_GPIO;
    conf.scl_io_num = I2C_SCL_GPIO;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.master.clk_speed = 100000;
    ESP_LOGI(TAG, "Setting up I2C");
    e = i2c_param_config(I2C_NUM_0, &conf);
    if(e == ESP_OK) {
        e = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
        if(e == ESP_OK) {
            // Init power management
            e = m5power_init();
            if(e == ESP_OK) {
                ESP_LOGD(TAG, "Power manager initialized");
            } else {
                ESP_LOGE(TAG, "Error initializing power manager");
                ++error_count;
            }
        } else {
            ESP_LOGE(TAG, "Error during I2C driver installation: %s", esp_err_to_name(e));
            ++error_count;
        }
    } else {
        ESP_LOGE(TAG, "Error setting up I2C: %s", esp_err_to_name(e));
        ++error_count;
    }

    // Init button
    e = m5button_init();
    if(e == ESP_OK) {
        ESP_LOGD(TAG, "Button initialized");
    } else {
        ESP_LOGE(TAG, "Error initializing button");
        ++error_count;
    }

    // Init display
    e = m5display_init();
    if(e == ESP_OK) {
        ESP_LOGD(TAG, "Display initialized");
    } else {
        ESP_LOGE(TAG, "Error initializing display");
        ++error_count;
    }

    if(error_count == 0) {
        ESP_LOGD(TAG, "M5StickC initialized successfully");
        return ESP_OK;
    } else {
        ESP_LOGE(TAG, "%d errors found while initializing M5StickC", error_count);
        return ESP_FAIL;
    }
}