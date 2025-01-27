#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "inc/SSD1306/ssd1306.h"
#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"

#define MULT_FREQUENCY_KHZ 1000
#define FREQUENCY_KHZ(x) (x * MULT_FREQUENCY_KHZ)

#define PIN_RAIN            26
#define PIN_SOIL            27
#define ADC_CHANNEL_RAIN     0
#define ADC_CHANNEL_SOIL     1
#define DELAY_READINGS      1000

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ADDRESS_OLED    0x3C
#define WIDTH_OLED      128
#define HEIGHT_OLED     64

char buffer_show[12];


int main()
{
    stdio_init_all();

    init_Lora();
    
    i2c_init(I2C_PORT, FREQUENCY_KHZ(400));
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_t oled;
    oled.external_vcc = false;
    ssd1306_init(&oled, WIDTH_OLED, HEIGHT_OLED, ADDRESS_OLED, I2C_PORT);
    ssd1306_clear(&oled);

    ssd1306_draw_string(&oled, 10, 31, 1.2, "Initializing...");
    ssd1306_show(&oled);
    sleep_ms(3000);

    while (true) {
        lora_rx();
        ssd1306_clear(&oled);
        if(rain_percent > 45) {
            sprintf(buffer_show, "Chuva Forte");
        }
        else if(rain_percent <= 45 && rain_percent > 25) {
            sprintf(buffer_show, "Chovendo");
        }
        else if(rain_percent <= 25) {
            sprintf(buffer_show, "Sem Chuva");
        }
        // sprintf(buffer_show, "Chuva: %d %%", rain_percent);
        ssd1306_draw_string(&oled, 25, 10, 1.2, buffer_show);
        sprintf(buffer_show, "Umidade solo: %d %%", humidity_percent);
        ssd1306_draw_string(&oled, 10, 31, 1.2, buffer_show);
        // if(temperature >) {
            
        // }
        sprintf(buffer_show, "Temperatura: %.1f C", temperature);
        ssd1306_draw_string(&oled, 10, 52, 1.2, buffer_show);
        ssd1306_show(&oled);
        sleep_ms(100);
    }
}