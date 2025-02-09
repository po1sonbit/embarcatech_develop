#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"
#include "hardware/timer.h"
#include "inc/SSD1306/ssd1306.h"
#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"

#include "inc/Mqtt_Wifi/mqtt_wifi.h"

#define HIGH                        1       // Representa o sinal no estado alto, para evitar magic numbers
#define LOW                         0       // Representa o sinal no estado baixo, para evitar magic numbers

#define MIN_CHUVA_FORTE             45
#define MIN_CHOVENDO                25

#define TIMER_FREQUENCY_MS          30000   // Tempo para chamar rotina de envio pro MQTT
#define WATCHDOG_TIMEOUT            8388    // Tempo para acionar o watchdog (MAX 8,3 seg)
#define FREQUENCY_LORA_RX           100     // Frequência de leitura do buffer Lora
#define TIMEOUT_LORA_NOPACKET__MS   10000   // Tempo para acionar alerta de ausência de pacotes recebidos

#define MULT_FREQUENCY_KHZ          1000    // Multiplicador de frequência
#define I2C_FREQUENCY_KHZ(x)        (x * MULT_FREQUENCY_KHZ)    // Função constante para frequência de I2C

#define I2C_PORT                    i2c1    // Seleção de I2C
#define I2C_SDA                     14      // GPIO SDA da I2C1
#define I2C_SCL                     15      // GPIO SCL da I2C1
#define ADDRESS_OLED                0x3C    // Endereço do display Oled
#define WIDTH_OLED                  128     // Largura da tela efetiva do display Oled
#define HEIGHT_OLED                 64      // Altura da tela efetiva do display Oled

char buffer_show[12];               // Vetor apenas para armazenar a string que será mostrada no 
volatile uint8_t blockRX = false;   // Variável de controle para alternar entre Lora e Wifi/MQTT
uint8_t wifi_status = false;        // Flag de controle para ativar e desativar Wifi
// Buffers para envio pro MQTT
char payload_soil_mqtt[6];
char payload_rain_mqtt[6];
char payload_temp_mqtt[8];

// Buffers para display Oled
char payload_soil_show[21];
char payload_rain_show[22];
char payload_temp_show[21];

void start_i2c(void);   // Configura I2C
void config_leds(void); // Configura Leds do RGB
bool send_to_mqtt_callback(struct repeating_timer *t);   // Função callback do timer de envio pro mqtt

int main()
{
    // Inicialização de periféricos
    stdio_init_all();
    
    // I2C
    start_i2c();    // Configuração do I2C
    
    // I2C - OLED
    ssd1306_t oled; // Struct Oled
    oled.external_vcc = false; // Não utiliza alimentação externa para o display (Geralmente se utiliza para displays maiores)
    ssd1306_init(&oled, WIDTH_OLED, HEIGHT_OLED, ADDRESS_OLED, I2C_PORT);
    ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display

    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
    ssd1306_draw_string(&oled, 12, 31, 1.2, "Inicializando...");    //Põe a string no buffer do display
    ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display
    sleep_ms(3000);

    ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
    ssd1306_draw_string(&oled, 6, 21, 1.2, "Configurando LEDs,");    //Põe a string no buffer do display
    ssd1306_draw_string(&oled, 4, 41, 1.2, "Timer e Watchdog...");    //Põe a string no buffer do display
    ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display

    // Timer
    struct repeating_timer handle_timer;    // Handle para referenciar o timer
    add_repeating_timer_ms(TIMER_FREQUENCY_MS, send_to_mqtt_callback, NULL, &handle_timer); // Inicializa o timer

    // LED RGB
    config_leds();  // Configuração Leds do RGB
    sleep_ms(3000);

    // Watchdog
    if (watchdog_caused_reboot()) { //Verifica se o reset do microcontrolador foi devido ao watchdog, ou seja, travamento no sistema
        printf("Rebooted by Watchdog!\n");
        ssd1306_clear(&oled);
        ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);
        ssd1306_draw_string(&oled, 30, 10, 1.2, "Resetado");
        ssd1306_draw_string(&oled, 20, 31, 1.2, "por Watchdog!");
        ssd1306_show(&oled);
    }
    
    watchdog_enable(WATCHDOG_TIMEOUT, true); // Inicializa o watchdog em 20 segundos
    watchdog_update(); // Reset do tempo do watchdog
    sleep_ms(3000);

    // SPI
    init_Lora();    // Configuração da SPI 0 para utilização do módulo Lora
    watchdog_update();  // Reset do tempo do watchdog
    ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
    ssd1306_draw_string(&oled, 25, 10, 1.2, "Iniciando");   //Põe a string no buffer do display
    ssd1306_draw_string(&oled, 12, 31, 1.2, "comunicacao Lora");    //Põe a string no buffer do display
    ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display
    sleep_ms(3000);

    ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
    ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
    ssd1306_draw_string(&oled, 33, 10, 1.2, "Esperando");   //Põe a string no buffer do display
    ssd1306_draw_string(&oled, 24, 31, 1.2, "comunicacao"); //Põe a string no buffer do display
    ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display

    absolute_time_t last_receive_time = get_absolute_time();    // Variável para controle de tempo sem receber pacotes Lora

    while (true) {
        if(blockRX) {   // Lora bloqueado para recepção, começa rotina de envio MQTT
            watchdog_update();  // Reset do tempo do watchdog
            last_receive_time = get_absolute_time();  // Atualiza o tempo da última mensagem lora recebida
            if(!wifi_status) { // Se Wifi não está ativo
                ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
                ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
                ssd1306_draw_string(&oled, 12, 21, 1.2, "Conectando Wifi");    //Põe a string no buffer do display
                ssd1306_draw_string(&oled, 25, 41, 1.2, "e MQTT");    //Põe a string no buffer do display
                ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display

                watchdog_update();  // Reset do tempo do watchdog
                start_Wifi(); // Inicializa Wifi
                start_MQTT(); // Inicializa MQTT
                wifi_status = true;
                sleep_ms(3000);
            }
            watchdog_update();  // Reset do tempo do watchdog

            sprintf(payload_soil_mqtt, "%d %%", humidity_percent);  // Função para conversão dos valores numericos em uma string para envio pro MQTT
            sprintf(payload_rain_mqtt, "%d %%", rain_percent);      // Função para conversão dos valores numericos em uma string para envio pro MQTT
            if(temperature >= SENSOR_ERROR) {

            }
            sprintf(payload_temp_mqtt, "%.1f C", temperature);      // Função para conversão dos valores numericos em uma string para envio pro MQTT

            last_receive_time = get_absolute_time();  // Atualiza o tempo da última mensagem lora recebida
            if(mqtt_client_is_connected(global_mqtt_client)) {
                ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
                ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
                ssd1306_draw_string(&oled, 2, 31, 1.2, "Enviando dados via MQTT");      //Põe a string no buffer do display
                ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display

                cyw43_arch_lwip_begin();    // Função para travar o mutex e garantir que múltiplas partes do código não acessem o wifi ao mesmo tempo
                err_t err = mqtt_publish(global_mqtt_client, TOPIC_SOIL, payload_soil_mqtt, strlen(payload_soil_mqtt), 1, 0, NULL, NULL);   // Publica pro Broker MQTT
                cyw43_arch_lwip_end();      // Destrava o mutex
                // Rotinas para modificar o log e o buffer que será mostrado no display
                if(err == ERR_OK) {
                    printf("Sended MQTT soil status: %s\n", payload_soil_mqtt);
                    sprintf(payload_soil_show, "Solo: Enviado");
                }
                else {
                    printf("FAILED to send MQTT soil status\n");
                    sprintf(payload_soil_show, "Solo: Erro ao enviar");
                }
                sleep_ms(200);
                cyw43_arch_lwip_begin();
                err = mqtt_publish(global_mqtt_client, TOPIC_RAIN, payload_rain_mqtt, strlen(payload_rain_mqtt), 1, 0, NULL, NULL);
                cyw43_arch_lwip_end();
                if(err == ERR_OK) {
                    printf("Sended MQTT rain status: %s\n", payload_rain_mqtt);
                    sprintf(payload_rain_show, "Chuva: Enviado");
                }
                else {
                    printf("FAILED to send MQTT rain status\n");
                    sprintf(payload_rain_show, "Chuva: Erro ao enviar");
                }
                sleep_ms(200);
                cyw43_arch_lwip_begin();
                err = mqtt_publish(global_mqtt_client, TOPIC_TEMP, payload_temp_mqtt, strlen(payload_temp_mqtt), 1, 0, NULL, NULL);
                cyw43_arch_lwip_end();
                if(err == ERR_OK) {
                    printf("Sended MQTT temp status: %s\n", payload_temp_mqtt);
                    sprintf(payload_temp_show, "Temp: Enviado");
                }
                else {
                    printf("FAILED to send MQTT temperature status\n");
                    sprintf(payload_temp_show, "Temp: Erro ao enviar");
                }

                watchdog_update();
                ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
                ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);    //Desenha um contorno em volta da tela
                ssd1306_draw_string(&oled, 2, 10, 1.2, payload_soil_show);    //Põe a string no buffer do display
                ssd1306_draw_string(&oled, 2, 31, 1.2, payload_rain_show);    //Põe a string no buffer do display
                ssd1306_draw_string(&oled, 2, 52, 1.2, payload_temp_show);    //Põe a string no buffer do display
                ssd1306_show(&oled);    //Printa na tela o que estiver no buffer do display
                sleep_ms(2000);

                blockRX = false;
            }
        }
        else {  // Lora liberado para recepção, começa rotina de recebimento de pacotes Lora
            if(wifi_status) { // Se Wifi está ativo
                stop_Wifi(); // Desliga wifi e mqtt
                wifi_status = false;
            }
            watchdog_update();
            uint8_t received = lora_rx();  // Verifica se há algum pacote Lora foi recebido
            watchdog_update();
            
            if (received == PACK_RECEIVED) {
                last_receive_time = get_absolute_time();  // Atualiza o tempo da última mensagem lora recebida
                
                // Exibe os dados recebidos
                ssd1306_clear(&oled);
                ssd1306_draw_empty_square(&oled, 0, 0, WIDTH_OLED - 1, HEIGHT_OLED - 1);

                // Dependendo dos valores de porcentagem da precipitação, printa o status real da chuva no display Oled
                if (rain_percent > MIN_CHUVA_FORTE) {
                    sprintf(buffer_show, "Chuva Forte");
                } else if (rain_percent > MIN_CHOVENDO) {
                    sprintf(buffer_show, "Chovendo");
                } else {
                    sprintf(buffer_show, "Sem Chuva");
                }

                ssd1306_draw_string(&oled, 25, 10, 1.2, buffer_show);
                sprintf(buffer_show, "Umidade solo: %d %%", humidity_percent);
                ssd1306_draw_string(&oled, 10, 31, 1.2, buffer_show);
                if(temperature >= SENSOR_ERROR) // Em caso do valor de temperatura vir 255, indica erro
                    sprintf(buffer_show, "Temperatura: Erro");
                else
                    sprintf(buffer_show, "Temperatura: %.1f C", temperature);
                ssd1306_draw_string(&oled, 10, 52, 1.2, buffer_show);
                ssd1306_show(&oled);
            }
            else {
                if (absolute_time_diff_us(last_receive_time, get_absolute_time()) > TIMEOUT_LORA_NOPACKET__MS * 1000) { //Se passou os 10 segundos, alerta o usuário
                    ssd1306_clear(&oled);   // Limpa a tela antes de cada nova página mostrada no display
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
}

bool send_to_mqtt_callback(struct repeating_timer *t) {
    blockRX = true;
    return true;
}

void start_i2c(void) {  // Inicializa I2C
    i2c_init(I2C_PORT, I2C_FREQUENCY_KHZ(400));
    
    // Seleciona os pinos para a função de I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    // Habilita pull up interno para os pinos da I2C
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
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
