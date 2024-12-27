#include <stdio.h>
#include "pico/stdlib.h"

#define RGB_Red 13
#define RGB_Blue 12
#define RGB_Green 11

int main()
{
    uint8_t odd = 0;
    uint8_t iter = 100;

    gpio_init(RGB_Red);
    gpio_set_dir(RGB_Red, GPIO_OUT);
    gpio_init(RGB_Green);
    gpio_set_dir(RGB_Green, GPIO_OUT);
    stdio_init_all();

    while(true) {
        iter++;
        odd = iter % 2;
        if(odd) {
            printf("Blinking red!\r\n");
            gpio_put(RGB_Red, true);
            sleep_ms(1000);
            gpio_put(RGB_Red, false);
            sleep_ms(1000);
        }
        else {
            gpio_put(RGB_Red, true);
            gpio_put(RGB_Green, true);
            sleep_ms(200);
            gpio_put(RGB_Red, false);
            gpio_put(RGB_Green, false);
            sleep_ms(200);
            gpio_put(RGB_Red, true);
            gpio_put(RGB_Green, true);
            sleep_ms(200);
            gpio_put(RGB_Red, false);
            gpio_put(RGB_Green, false);
            sleep_ms(200);
        }
    }

}
