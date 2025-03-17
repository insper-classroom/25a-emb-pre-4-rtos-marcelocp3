#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include "pico/stdlib.h"
#include <stdio.h>

#define BTN_PIN_R 28
#define LED_PIN_R 4
#define BTN_PIN_G 26
#define LED_PIN_G 6

QueueHandle_t xQueueButId_r;
SemaphoreHandle_t xSemaphore_r;
QueueHandle_t xQueueButId_g;
SemaphoreHandle_t xSemaphore_g;

void btn_callback_r(uint gpio, uint32_t events) {
    if (events == 0x4) {
        xSemaphoreGiveFromISR(xSemaphore_r, 0);
    }
}

void btn_callback_g(uint gpio, uint32_t events) {
    if (events == 0x4) {
        xSemaphoreGiveFromISR(xSemaphore_g, 0);
    }
}

void global_callback(uint gpio, uint32_t events) {
    if (gpio == BTN_PIN_R) {
        btn_callback_r(gpio, events);
    } else if (gpio == BTN_PIN_G) {
        btn_callback_g(gpio, events);
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId_r, &delay, 0)) {
            printf("%d\n", delay);
        }
        if (delay > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    int delay = 0;
    while (true) {
        if (xQueueReceive(xQueueButId_g, &delay, 0)) {
            printf("%d\n", delay);
        }
        if (delay > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(delay));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
    }
}

void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &global_callback);
    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000) delay += 100; else delay = 100;
            printf("delay btn %d\n", delay);
            xQueueSend(xQueueButId_r, &delay, 0);
        }
    }
}

void btn_2_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true, &global_callback);
    int delay = 0;
    while (true) {
        if (xSemaphoreTake(xSemaphore_g, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (delay < 1000) delay += 100; else delay = 100;
            printf("delay btn %d\n", delay);
            xQueueSend(xQueueButId_g, &delay, 0);
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS\n");
    xQueueButId_r = xQueueCreate(32, sizeof(int));
    xSemaphore_r = xSemaphoreCreateBinary();
    xQueueButId_g = xQueueCreate(32, sizeof(int));
    xSemaphore_g = xSemaphoreCreateBinary();
    xTaskCreate(led_1_task, "LED_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task_G", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task_G", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    while (true);
}
