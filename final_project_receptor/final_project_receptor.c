#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"

#include "inc/SSD1306/ssd1306.h"
#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"

#define HIGH    1
#define LOW     0

#define WATCHDOG_TIMEOUT        10000
#define FREQUENCY_LORA_RX       100
#define TIMEOUT_LORA_MS         20000

#define MULT_FREQUENCY_KHZ      1000
#define I2C_FREQUENCY_KHZ(x)    (x * MULT_FREQUENCY_KHZ)

#define I2C_PORT                i2c1
#define I2C_SDA                 14
#define I2C_SCL                 15
#define ADDRESS_OLED            0x3C
#define WIDTH_OLED              128
#define HEIGHT_OLED             64

char buffer_show[12];

void start_i2c(void);

int main()
{
    stdio_init_all();
    
    start_i2c();

    gpio_init(PIN_LED_RED);
    gpio_set_dir(PIN_LED_RED, GPIO_OUT);
    gpio_pull_up(PIN_LED_RED);
    
    ssd1306_t oled;
    oled.external_vcc = false;
    ssd1306_init(&oled, WIDTH_OLED, HEIGHT_OLED, ADDRESS_OLED, I2C_PORT);
    ssd1306_clear(&oled);

    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);
    ssd1306_draw_string(&oled, 12, 31, 1.2, "Inicializando...");
    ssd1306_show(&oled);

    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
        ssd1306_clear(&oled);
        ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);
        ssd1306_draw_string(&oled, 30, 10, 1.2, "Resetado");
        ssd1306_draw_string(&oled, 20, 31, 1.2, "por Watchdog!");
        ssd1306_show(&oled);
    }
    
    watchdog_enable(WATCHDOG_TIMEOUT, 1);
    watchdog_update();
    sleep_ms(3000);

    init_Lora();
    ssd1306_clear(&oled);
    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);
    ssd1306_draw_string(&oled, 25, 10, 1.2, "Iniciando");
    ssd1306_draw_string(&oled, 12, 31, 1.2, "comunicacao Lora");
    ssd1306_show(&oled);
    sleep_ms(3000);

    ssd1306_clear(&oled);
    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);
    ssd1306_draw_string(&oled, 33, 10, 1.2, "Esperando");
    ssd1306_draw_string(&oled, 24, 31, 1.2, "comunicacao");
    ssd1306_show(&oled);

    absolute_time_t last_receive_time = get_absolute_time();

    while (true) {
        uint8_t received = lora_rx();  // Tente receber algo do LoRa
        
        if (received == PACK_RECEIVED) {
            last_receive_time = get_absolute_time();  // Atualiza o tempo da última mensagem recebida
            
            // Exibe os dados recebidos
            ssd1306_clear(&oled);
            ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);

            if (rain_percent > 45) {
                sprintf(buffer_show, "Chuva Forte");
            } else if (rain_percent > 25) {
                sprintf(buffer_show, "Chovendo");
            } else {
                sprintf(buffer_show, "Sem Chuva");
            }

            ssd1306_draw_string(&oled, 25, 10, 1.2, buffer_show);
            sprintf(buffer_show, "Umidade solo: %d %%", humidity_percent);
            ssd1306_draw_string(&oled, 10, 31, 1.2, buffer_show);
            sprintf(buffer_show, "Temperatura: %.1f C", temperature);
            ssd1306_draw_string(&oled, 10, 52, 1.2, buffer_show);
            ssd1306_show(&oled);
        }
        else {
            if (absolute_time_diff_us(last_receive_time, get_absolute_time()) > TIMEOUT_LORA_MS * 1000) {
                ssd1306_clear(&oled);
                ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);
                ssd1306_draw_string(&oled, 8, 20, 1.2, "Transmissor fora do");
                ssd1306_draw_string(&oled, 8, 40, 1.2, "range ou desligado");
                ssd1306_show(&oled);
                gpio_put(PIN_LED_RED, HIGH);
                sleep_ms(500);
                gpio_put(PIN_LED_RED, LOW);
            }
        }

        watchdog_update();
        sleep_ms(FREQUENCY_LORA_RX);
    }
}

void start_i2c(void) {
    i2c_init(I2C_PORT, I2C_FREQUENCY_KHZ(400));
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}
