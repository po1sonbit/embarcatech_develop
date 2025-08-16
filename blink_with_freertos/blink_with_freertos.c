#include <stdio.h>
#include "pico/stdlib.h"
#include <FreeRTOS.h>
#include <task.h>
#include "mapping_pin_bitdog.h"

void vBlinkTest();

int main()
{
    stdio_init_all();
    gpio_init(PIN_LED_BLUE);
    gpio_set_dir(PIN_LED_BLUE, GPIO_OUT);

    xTaskCreate(vBlinkTest, "Blink task w freertos", 128, NULL, 1, NULL);
    vTaskStartScheduler();
}

void vBlinkTest() {
    while(true) {
        gpio_put(PIN_LED_BLUE, 1);
        vTaskDelay(250/portTICK_PERIOD_MS);
        gpio_put(PIN_LED_BLUE, 0);
        vTaskDelay(250/portTICK_PERIOD_MS);
    }
}