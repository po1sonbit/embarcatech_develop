#include <stdio.h>
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include <task.h>
#include "mapping_pin_bitdog.h"

/*
*   Testado na placa bitdoglab
*/

uint8_t button_state = 0;
uint8_t led_state = 0;

TaskHandle_t led_task_handle;

uint8_t read_button(void);
void control_led(uint8_t _led_state);
void button_task(void *arg);
void process_button_task(void *arg);
void led_task(void *arg);

int main()
{
    stdio_init_all();

    gpio_init(PIN_LED_GREEN);
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    
    gpio_pull_up(BUTTON_A_PIN);

    xTaskCreate(button_task, "Button Task", 128, NULL, 1, NULL);
    xTaskCreate(process_button_task, "Process Button Task", 128, NULL, 2, NULL);
    xTaskCreate(led_task, "LED Task", 128, NULL, 3, &led_task_handle);

    vTaskStartScheduler();
    return 0;
}

void button_task(void *arg) {
    while(true) {
        button_state = read_button();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void process_button_task(void *arg) {
    while(true) {
        if(button_state)
            xTaskNotifyGive(led_task_handle);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void led_task(void *arg) {
    while(true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if(!button_state)
            led_state = 1;
        else
            led_state = 0;
        control_led(led_state);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

uint8_t read_button(void) {
    return (uint8_t)gpio_get(BUTTON_A_PIN);
}

void control_led(uint8_t _led_state) {
    gpio_put(PIN_LED_GREEN, _led_state);
}