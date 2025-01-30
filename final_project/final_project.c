#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "inc/SSD1306/ssd1306.h"
#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"
#include "inc/Mqtt_Wifi/mqtt_wifi.h"

#include "pico/multicore.h"
#include "pico/mutex.h"

#define HIGH    1
#define LOW     0

#define TIMER_FREQUENCY_MS    100

#define MULT_FREQUENCY_KHZ  1000
#define FREQUENCY_KHZ(x)    (x * MULT_FREQUENCY_KHZ)

#define PIN_RAIN            26
#define PIN_SOIL            27
#define ADC_CHANNEL_RAIN     0
#define ADC_CHANNEL_SOIL     1
#define DELAY_READINGS      1000

#define I2C_PORT            i2c1
#define I2C_SDA             14
#define I2C_SCL             15
#define ADDRESS_OLED        0x3C
#define WIDTH_OLED          128
#define HEIGHT_OLED         64

char buffer_show[12];
volatile uint8_t readyRX = false;
char payload_soil[2];
char payload_rain[2];
char payload_temp[5];

void core1_process(void);

int main()
{
    stdio_init_all();

    multicore_launch_core1(core1_process);

    init_Lora();
    
    i2c_init(I2C_PORT, FREQUENCY_KHZ(400));
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    struct repeating_timer handle_timer;
    add_repeating_timer_ms(TIMER_FREQUENCY_MS, routine_lora_oled_callback, NULL, &handle_timer);
    
    ssd1306_t oled;
    oled.external_vcc = false;
    ssd1306_init(&oled, WIDTH_OLED, HEIGHT_OLED, ADDRESS_OLED, I2C_PORT);
    ssd1306_clear(&oled);

    ssd1306_draw_string(&oled, 10, 31, 1.2, "Initializing...");
    ssd1306_show(&oled);

    start_Wifi();
    start_MQTT();
    sleep_ms(3000);

    while (true) {
        if(readyRX) {
            readyRX = false;
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
        }
    }
}

void core1_process(void) {
    while (true) {
        sprintf(payload_soil, "%d", humidity_percent);
        sprintf(payload_rain, "%d", rain_percent);
        sprintf(payload_temp, "%.1f", temperature);

        if(global_mqtt_client && mqtt_client_is_connected(global_mqtt_client)) {
            err_t err = mqtt_publish(global_mqtt_client, TOPIC_SOIL, payload_soil, strlen(payload_soil), 1, 0, NULL, NULL);
            if(err == ERR_OK)
                printf("Sended MQTT soil status: %s\n", payload_soil);
            else
                printf("FAILED to send MQTT soil status\n");
            err = mqtt_publish(global_mqtt_client, TOPIC_SOIL, payload_rain, strlen(payload_rain), 1, 0, NULL, NULL);
            if(err == ERR_OK)
                printf("Sended MQTT rain status: %s\n", payload_rain);
            else
                printf("FAILED to send MQTT rain status\n");
            err = mqtt_publish(global_mqtt_client, TOPIC_SOIL, payload_temp, strlen(payload_temp), 1, 0, NULL, NULL);
            if(err == ERR_OK)
                printf("Sended MQTT temp status: %s\n", payload_temp);
            else
                printf("FAILED to send MQTT temperature status\n");
        }
        sleep_ms(5000);
    }
}

uint8_t routine_lora_oled_callback(struct repeating_timer *t) {
  readyRX = true;
  return true;
}