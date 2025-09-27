#include "MQTT_homework3_4.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "inc/SSD1306/ssd1306.h"
#include "inc/Mqtt_Wifi/mqtt_wifi.h"
#include "inc/bmp280/bmp280.h"

uint16_t raw_x = 0;
uint16_t raw_y = 0;
int32_t raw_temperature;
int32_t raw_pressure;
struct bmp280_calib_param params;

// Buffers para envio pro MQTT
char payload_temp_mqtt[8];
char payload_press_mqtt[8];
char payload_x_axis_mqtt[8];

// Buffers para display Oled
char payload_temp_show[21];
char payload_press_show[23];
char payload_x_axis_show[22];
char payload_led_blue_show[19];

void start_i2c(void);
void config_leds(void);

int main()
{
    stdio_init_all(); 

    adc_init();

    adc_gpio_init(ADC_PIN_JOYSTICK_X);
    adc_gpio_init(ADC_PIN_JOYSTICK_Y);

    // I2C
    start_i2c();    // Configuração do I2C

    bmp280_init();
    bmp280_get_calib_params(&params);
    
    // I2C - OLED
    ssd1306_t oled; // Struct Oled
    oled.external_vcc = false; // Não utiliza alimentação externa para o display (Geralmente se utiliza para displays maiores)
    ssd1306_init(&oled, WIDTH_OLED, HEIGHT_OLED, ADDRESS_OLED, I2C_PORT_OLED);
    ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display

    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
    ssd1306_draw_string(&oled, 12, 31, 1.2, "Inicializando...");    //Põe a string no buffer do display
    ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display
    sleep_ms(3000);

    // LED RGB
    config_leds();  // Configuração Leds do RGB

    start_Wifi(); // Inicializa Wifi
    start_MQTT(); // Inicializa MQTT
    sleep_ms(3000);

    mqtt_subscribe(global_mqtt_client, TOPIC_SUB_LED, 1, mqtt_sub_request_cb, NULL);

    while (true) {
        if(mqtt_client_is_connected(global_mqtt_client)) {
            printf("entrou\n");
            bmp280_read_raw(&raw_temperature, &raw_pressure);
            int32_t temperature = bmp280_convert_temp(raw_temperature, &params);
            float temperature_f = temperature / 100.0;
            int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temperature, &params);
            float pressure_f = pressure / 1000.0;
            printf("Pressure = %.3f kPa\n", pressure_f);
            printf("Temp. = %.1f C\n", temperature_f);

            sprintf(payload_temp_mqtt, "Temperatura: %.1f C", temperature_f);      // Função para conversão dos valores numericos em uma string para envio pro MQTT
            sprintf(payload_press_mqtt, "Pressão: %.3f kpA", pressure_f);      // Função para conversão dos valores numericos em uma string para envio pro MQTT

            // Lê joystick
            adc_select_input(ADC_CHANNEL_JOYSTICK_X);
            sleep_us(2);
            raw_x = adc_read();
            adc_select_input(ADC_CHANNEL_JOYSTICK_Y);
            sleep_us(2);
            raw_y = adc_read();
            printf("Eixo X = %d\n", raw_x);
            int pct_x = (raw_x * 100) / 4095;

            sprintf(payload_x_axis_mqtt, "Ajuste %%: %d%%", pct_x);

            cyw43_arch_lwip_begin();    // Função para travar o mutex e garantir que múltiplas partes do código não acessem o wifi ao mesmo tempo
            err_t err = mqtt_publish(global_mqtt_client, TOPIC_TEMP, payload_temp_mqtt, strlen(payload_temp_mqtt), 1, 0, NULL, NULL);   // Publica pro Broker MQTT
            cyw43_arch_lwip_end();      // Destrava o mutex
            // Rotinas para modificar o log e o buffer que será mostrado no display
            if(err == ERR_OK) {
                printf("Sended MQTT temperature: %s\n", payload_temp_mqtt);
                sprintf(payload_temp_show, "Temperatura: %.1fC", temperature_f);
            }
            else {
                printf("FAILED to send MQTT temperature status\n");
                sprintf(payload_temp_show, "Temperatura: Erro ao enviar");
            }
            // sleep_ms(200);
            cyw43_arch_lwip_begin();
            err = mqtt_publish(global_mqtt_client, TOPIC_PRESS, payload_press_mqtt, strlen(payload_press_mqtt), 1, 0, NULL, NULL);
            cyw43_arch_lwip_end();
            if(err == ERR_OK) {
                printf("Sended MQTT pressure: %s\n", payload_press_mqtt);
                sprintf(payload_press_show, "Pressao: %.2fkPA", pressure_f);
            }
            else {
                printf("FAILED to send MQTT temperature status\n");
                sprintf(payload_press_show, "Pressao: Erro ao enviar");
            }

            cyw43_arch_lwip_begin();    // Função para travar o mutex e garantir que múltiplas partes do código não acessem o wifi ao mesmo tempo
            err = mqtt_publish(global_mqtt_client, TOPIC_X_AXIS, payload_x_axis_mqtt, strlen(payload_x_axis_mqtt), 1, 0, NULL, NULL);   // Publica pro Broker MQTT
            cyw43_arch_lwip_end();      // Destrava o mutex
            // Rotinas para modificar o log e o buffer que será mostrado no display
            if(err == ERR_OK) {
                printf("Sended MQTT X axis: %s\n", payload_x_axis_mqtt);
                sprintf(payload_x_axis_show, "X axis: %d%%", pct_x);
            }
            else {
                printf("FAILED to send MQTT X axis status\n");
                sprintf(payload_x_axis_show, "X axis: Erro ao enviar");
            }

            if(led_blue_status)
                sprintf(payload_led_blue_show, "LED Azul: LIGADO");
            else
                sprintf(payload_led_blue_show, "LED Azul: DESLIGADO");

            ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
            ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
            ssd1306_draw_string(&oled, 2, 10, 1.2, payload_temp_show);    //Põe a string no buffer do display
            ssd1306_draw_string(&oled, 2, 22, 1.2, payload_press_show);    //Põe a string no buffer do display
            ssd1306_draw_string(&oled, 2, 32, 1.2, payload_x_axis_show);    //Põe a string no buffer do display
            ssd1306_draw_string(&oled, 2, 42, 1.2, payload_led_blue_show);    //Põe a string no buffer do display
            ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display
            // sleep_ms(1000);
        }
        sleep_ms(1000);
    }
}

void start_i2c(void) {  // Inicializa I2C
    i2c_init(I2C_PORT_OLED, I2C_FREQUENCY_KHZ(400));
    
    // Seleciona os pinos para a função de I2C
    gpio_set_function(I2C_SDA_OLED, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_OLED, GPIO_FUNC_I2C);
    // Habilita pull up interno para os pinos da I2C
    gpio_pull_up(I2C_SDA_OLED);
    gpio_pull_up(I2C_SCL_OLED);

    i2c_init(I2C_PORT, I2C_FREQUENCY_KHZ(100));
    
    // Seleciona os pinos para a função de I2C
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    // Habilita pull up interno para os pinos da I2C
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

void config_leds(void) {
    // Inicializa GPIOs
    gpio_init(PIN_LED_BLUE);
    gpio_init(PIN_LED_GREEN);
    gpio_init(PIN_LED_RED);
    // Define como Output as GPIOS dos leds
    gpio_set_dir(PIN_LED_BLUE, GPIO_OUT);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);
    gpio_set_dir(PIN_LED_RED, GPIO_OUT);
    // Habilita pull up interno para os leds
    gpio_pull_up(PIN_LED_BLUE);
    gpio_pull_up(PIN_LED_GREEN);
    gpio_pull_up(PIN_LED_RED);
}