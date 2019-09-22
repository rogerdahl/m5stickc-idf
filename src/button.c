/**
 * button.c
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#include "button.h"

static const char * TAG = "button";

ESP_EVENT_DEFINE_BASE(M5STICKC_BUTTON_EVENT_BASE);

StaticEventGroup_t m5stickc_button_event_group_buffer;
EventGroupHandle_t m5stickc_button_event_group;

void IRAM_ATTR m5stickc_button_A_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(gpio_get_level(M5STICKC_BUTTON_A_GPIO) == 0) {
        xEventGroupSetBitsFromISR(m5stickc_button_event_group, M5STICKC_BUTTON_A_PUSH_BIT, &xHigherPriorityTaskWoken);
    } else {
        xEventGroupSetBitsFromISR(m5stickc_button_event_group, M5STICKC_BUTTON_A_POP_BIT, &xHigherPriorityTaskWoken);
    }
}

esp_err_t m5stickc_button_init() {
    esp_err_t e;

    m5stickc_button_event_group = xEventGroupCreateStatic(&m5stickc_button_event_group_buffer);
    if(m5stickc_button_event_group == NULL) {
        ESP_LOGE(TAG, "Error creating button event group");
        return ESP_FAIL;
    }

    gpio_pad_select_gpio(M5STICKC_BUTTON_A_GPIO);
    gpio_set_direction(M5STICKC_BUTTON_A_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(M5STICKC_BUTTON_A_GPIO, GPIO_INTR_ANYEDGE);
    #define ESP_INTR_FLAG_DEFAULT 0
    e = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    if(e == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "ISR service already installed");
    } else if(e == ESP_ERR_NO_MEM || e == ESP_ERR_NOT_FOUND) {
        ESP_LOGE(TAG, "Error installing ISR service");
        return ESP_FAIL;
    }
    gpio_isr_handler_add(M5STICKC_BUTTON_A_GPIO, m5stickc_button_A_isr_handler, NULL);

    xTaskCreate(&m5stickc_button_task, "button_task", configMINIMAL_STACK_SIZE*10, NULL, 20, NULL);

    ESP_LOGD(TAG, "Button initialized");

    return ESP_OK;
}

void m5stickc_button_task(void * pvParameter) {
    EventBits_t event;

    while(1) {
        event = xEventGroupWaitBits(m5stickc_button_event_group, M5STICKC_BUTTON_A_PUSH_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if((event & M5STICKC_BUTTON_A_PUSH_BIT) != 0) {
            vTaskDelay(M5STICKC_BUTTON_DEBOUNCE_TIME/portTICK_PERIOD_MS);
            xEventGroupClearBits(m5stickc_button_event_group, M5STICKC_BUTTON_A_POP_BIT);
            event = xEventGroupWaitBits(m5stickc_button_event_group, M5STICKC_BUTTON_A_POP_BIT, pdTRUE, pdFALSE, M5STICKC_BUTTON_HOLD_TIME / portTICK_PERIOD_MS);
            if((event & M5STICKC_BUTTON_A_POP_BIT) != 0) {
                esp_event_post_to(m5stickc_event_loop, M5STICKC_BUTTON_EVENT_BASE, M5STICKC_BUTTON_A_CLICK, NULL, 0, portMAX_DELAY);
                ESP_LOGD(TAG, "BUTTON_A_CLICK event");
            } else {
                esp_event_post_to(m5stickc_event_loop, M5STICKC_BUTTON_EVENT_BASE, M5STICKC_BUTTON_A_HOLD, NULL, 0, portMAX_DELAY);
                ESP_LOGD(TAG, "BUTTON_A_HOLD event");
            }
        }
    }
}