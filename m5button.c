/**
 * m5button.c
 * 
 * (C) 2019 - Pablo Bacho <pablo@pablobacho.com>
 * This code is licensed under the MIT License.
 */

#include "m5button.h"

static const char * TAG = "m5button";

ESP_EVENT_DEFINE_BASE(M5BUTTON_EVENT_BASE);

#if defined(CONFIG_SUPPORT_STATIC_ALLOCATION)
StaticEventGroup_t m5button_event_group_buffer;
StaticTask_t m5button_task_buffer;
StackType_t m5button_task_stack[];
#endif

EventGroupHandle_t m5button_event_group;

void IRAM_ATTR m5button_buttonA_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(gpio_get_level(M5BUTTON_BUTTON_A_GPIO) == 0) {
        xEventGroupSetBitsFromISR(m5button_event_group, M5BUTTON_BUTTON_A_PUSH_BIT, &xHigherPriorityTaskWoken);
    } else {
        xEventGroupSetBitsFromISR(m5button_event_group, M5BUTTON_BUTTON_A_POP_BIT, &xHigherPriorityTaskWoken);
    }
}

esp_err_t m5button_init() {
    esp_err_t e;

    #if defined(CONFIG_SUPPORT_STATIC_ALLOCATION)
    m5button_event_group = xEventGroupCreateStatic(&m5button_event_group_buffer);
    #else
    m5button_event_group = xEventGroupCreate();
    #endif
    if(m5button_event_group == NULL) {
        ESP_LOGE(TAG, "Error creating button event group");
        return ESP_FAIL;
    }

    gpio_pad_select_gpio(M5BUTTON_BUTTON_A_GPIO);
    gpio_set_direction(M5BUTTON_BUTTON_A_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(M5BUTTON_BUTTON_A_GPIO, GPIO_INTR_ANYEDGE);
    #define ESP_INTR_FLAG_DEFAULT 0
    e = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    if(e == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "ISR service already installed");
    } else if(e == ESP_ERR_NO_MEM || e == ESP_ERR_NOT_FOUND) {
        ESP_LOGE(TAG, "Error installing ISR service");
        return ESP_FAIL;
    }
    gpio_isr_handler_add(M5BUTTON_BUTTON_A_GPIO, m5button_buttonA_isr_handler, NULL);

    #if defined(CONFIG_SUPPORT_STATIC_ALLOCATION)
    TaskHandle_t task = xTaskCreateStatic(&m5button_task, "button_task", M5BUTTON_TASK_STACK_DEPTH, NULL, 20, m5button_task_stack, &m5button_task_buffer);
    if(task == NULL) {
        ESP_LOGE(TAG, "Error creating button_task");
        gpio_isr_handler_remove(M5BUTTON_BUTTON_A_GPIO);
        return ESP_FAIL;
    }
    #else
    BaseType_t r = xTaskCreate(&m5button_task, "button_task", M5BUTTON_TASK_STACK_DEPTH, NULL, 20, NULL);
    if(r != pdPASS) {
        ESP_LOGE(TAG, "Error creating button_task");
        gpio_isr_handler_remove(M5BUTTON_BUTTON_A_GPIO);
        return ESP_FAIL;
    }
    #endif

    ESP_LOGD(TAG, "Button initialized");
    return ESP_OK;
}

void m5button_task(void * pvParameter) {
    EventBits_t event;

    while(1) {
        event = xEventGroupWaitBits(m5button_event_group, M5BUTTON_BUTTON_A_PUSH_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
        if((event & M5BUTTON_BUTTON_A_PUSH_BIT) != 0) {
            vTaskDelay(M5BUTTON_DEBOUNCE_TIME/portTICK_PERIOD_MS);
            xEventGroupClearBits(m5button_event_group, M5BUTTON_BUTTON_A_POP_BIT);
            event = xEventGroupWaitBits(m5button_event_group, M5BUTTON_BUTTON_A_POP_BIT, pdTRUE, pdFALSE, M5BUTTON_HOLD_TIME / portTICK_PERIOD_MS);
            if((event & M5BUTTON_BUTTON_A_POP_BIT) != 0) {
                esp_event_post_to(m5_event_loop, M5BUTTON_EVENT_BASE, M5BUTTON_BUTTON_A_CLICK, NULL, 0, portMAX_DELAY);
                ESP_LOGD(TAG, "BUTTON_A_CLICK event");
            } else {
                esp_event_post_to(m5_event_loop, M5BUTTON_EVENT_BASE, M5BUTTON_BUTTON_A_HOLD, NULL, 0, portMAX_DELAY);
                ESP_LOGD(TAG, "BUTTON_A_HOLD event");
            }
        }
    }
}