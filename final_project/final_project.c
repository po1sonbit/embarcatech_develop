#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "inc/SSD1306/ssd1306.h"
#include "inc/Lora/lora_sx1280.h"

#define MULT_FREQUENCY_KHZ 1000
#define FREQUENCY_KHZ(x) (x * MULT_FREQUENCY_KHZ)

#define ADC_CHANNEL_RAIN     1
#define ADC_CHANNEL_SOIL     2
#define DELAY_READINGS      1000
// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
// #define SPI_PORT spi0
// #define PIN_MISO 16
// #define PIN_CS   17
// #define PIN_SCK  18
// #define PIN_MOSI 19

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

#define ADDRESS_OLED    0x3C
#define WIDTH_OLED      128
#define HEIGHT_OLED     64

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Put your timeout handler code in here
    return 0;
}



int main()
{
    stdio_init_all();

    // adc_init();
    // adc_select_input(ADC_CHANNEL_RAIN);
    // adc_init();
    // adc_select_input(ADC_CHANNEL_SOIL);

    // SPI initialisation. This example will use SPI at 1MHz.
    // spi_init(SPI_PORT, 1000*1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // spi_init(SPI_PORT, 1000*1000);
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    init_Lora();
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    // gpio_set_dir(PIN_CS, GPIO_OUT);
    // gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, FREQUENCY_KHZ(400));
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    
    ssd1306_t oled;
    oled.external_vcc = false;
    ssd1306_init(&oled, WIDTH_OLED, HEIGHT_OLED, ADDRESS_OLED, I2C_PORT);
    ssd1306_clear(&oled);

    // Timer example code - This example fires off the callback after 2000ms
    add_alarm_in_ms(2000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    while (true) {
        // lora_rx();
        // rainAnalog = analogRead(AO_PIN);
        // rainVoltage = rainAnalog * (5.0 / 1023.0);
        // int percent_soilmoisture = map(leitura_umid, 0, 2950, 100, 0);
        printf("Hello, world!\n");
        ssd1306_draw_string(&oled, 20, 0, 1, "Hello World!");
        ssd1306_draw_string(&oled, 20, 21, 1, "Hello World!");
        ssd1306_draw_string(&oled, 20, 42, 1, "Hello World!");
        ssd1306_show(&oled);
        sleep_ms(1000);
    }
}
