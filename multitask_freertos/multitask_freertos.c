#include <stdio.h>
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include <task.h>
#include "mapping_pin_bitdog.h"

uint8_t pin_led_green = PIN_LED_GREEN;
uint8_t pin_led_red = PIN_LED_RED;

void vBlinkTest(void *pvParameters);

int main()
{
    stdio_init_all();
    gpio_init(pin_led_green);
    gpio_init(pin_led_red);
    gpio_set_dir(pin_led_green, GPIO_OUT);
    gpio_set_dir(pin_led_red, GPIO_OUT);

    xTaskCreate(vBlinkTest, "Blink task w freertos", 128, (void *)&pin_led_green, 1, NULL);
    xTaskCreate(vBlinkTest, "Blink task w freertos 2", 128, (void *)&pin_led_red, 1, NULL);
    vTaskStartScheduler();
}

void vBlinkTest(void *pvParameters) {

    uint8_t *led_pin = (uint8_t *)pvParameters;

    while(true) {
        gpio_put(*led_pin, 1);
        vTaskDelay(250/portTICK_PERIOD_MS);
        gpio_put(*led_pin, 0);
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
}