#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"
#include <dht.h>

#define SENSOR_ERROR            255         // Macro de erro
// #define DEBUG    // descomentar para analisar logs dos ADC's
#define TIMER_FREQUENCY_MS      3000        // Frequência do timer
#define WATCHDOG_TIMEOUT        10000       // Tempo para acionar o watchdog

#define PIN_RAIN                26          // GPIO sensor de chuva
#define PIN_SOIL                27          // GPIO sensor de umidade do solo
#define PIN_DHT                 15          // GPIO sensor de temperatura e umidade do ar

#define PIO                     pio0        // Canal PIO
#define INTERNAL_PULL_UP_DHT    true        // Pull up interno para DHT11

#define ADC_CHANNEL_RAIN        0           // Canal ADC sensor de chuva
#define ADC_CHANNEL_SOIL        2           // Canal ADC umidade do solo

static const dht_model_t DHT_MODEL = DHT11; // Enum que indica o modelo do sensor

int soilAnalog = 0;                     // Valor ADC de sensor de umidade do solo
int rainAnalog = 0;                     // Valor ADC de sensor de chuva
uint8_t percent_rain = 0;               // Porcentagem de água no sensor, para incidência de chuva
uint8_t percent_humidity_soil = 0;      // Porcentagem de umidadade do solo
float temperature = 0.0;                // Valor float da temperatura
float humidity = 0.0;                   // Valor float da umidade do ar (Não utilizado)

dht_t DHT;                              // Struct do sensor DHT

uint8_t read_rain(void);
uint8_t read_soil_humidity(void);

bool read_sensors_transmit(struct repeating_timer *t);

int main()
{
    // Inicialização de periféricos
    stdio_init_all();

    // ADC
    adc_init();
    adc_gpio_init(PIN_RAIN);
    adc_gpio_init(PIN_SOIL);

    // PIO
    dht_init(&DHT, DHT_MODEL, PIO, PIN_DHT, INTERNAL_PULL_UP_DHT);

    // SPI
    init_Lora();

    // Timer
    struct repeating_timer handle_timer;
    add_repeating_timer_ms(TIMER_FREQUENCY_MS, read_sensors_transmit, NULL, &handle_timer);

    // Watchdog
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
    }
    
    watchdog_enable(WATCHDOG_TIMEOUT, true); // Inicializa o watchdog em 10 segundos
    watchdog_update();  // Reset do tempo do watchdog

    ///////////////////////////////////////
    

    while (true) {
        sleep_ms(1);
    }
}

bool read_sensors_transmit(struct repeating_timer *t) {
    percent_humidity_soil = read_soil_humidity();   // Recebe porcentagem da umidade do solo
    if(percent_humidity_soil != SENSOR_ERROR) {     // Há a possibilidade do sensor estar desconectado e o valor da porcentagem aparecer fixamente 100%, isso é devido a característica do ADC, o valor dele deve estar fixamente próximo de 0
        printf("Humidity Soil percentage: %d %%\n", percent_humidity_soil);
    }
    watchdog_update();  // Reset do tempo do watchdog

    percent_rain = read_rain(); // Recebe porcentagem do nível de chuva
    if(percent_rain != SENSOR_ERROR) {  // Há a possibilidade do sensor estar desconectado e o valor da porcentagem aparecer fixamente 100%, isso é devido a característica do ADC, o valor dele deve estar fixamente próximo de 0
        printf("Rain percentage: %d %%\n", percent_rain);
    }
    watchdog_update();  // Reset do tempo do watchdog
    
    dht_start_measurement(&DHT); // Utiliza-se também DMA para a coleta dos dados desse sensor, para que não ocupe processamento da CPU principal
    dht_result_t measure = dht_finish_measurement_blocking(&DHT, &humidity, &temperature);
    watchdog_update();  // Reset do tempo do watchdog
    if (measure == DHT_RESULT_OK) {
        printf("%.1f C\n", temperature);
    }
    else if (measure == DHT_RESULT_TIMEOUT) { // Sensor não comunica
        temperature = SENSOR_ERROR;
        printf("DHT sensor not responding. Please check your wiring.\n");
    } 
    else {  // Sensor com checksum errado, indica erros na transmissão dos dados do sensor
        assert(measure == DHT_RESULT_BAD_CHECKSUM);
        temperature = SENSOR_ERROR;
        printf("Bad checksum\n");
    }
    lora_tx(temperature, percent_humidity_soil, percent_rain); // Função para envio do pacote de dados via Lora
    watchdog_update();  // Reset do tempo do watchdog
    return 1;
}

uint8_t read_rain(void) {
    adc_select_input(ADC_CHANNEL_RAIN); // Seleção do ADC que será utilizado
    rainAnalog = adc_read();    // Leitura do ADC
    #ifdef DEBUG
    printf("adc RAIN: %d\n", rainAnalog);
    #endif
    if (rainAnalog < 0 || rainAnalog > 4096) {  //Problema com o pino do microcontrolador ou com o sensor
        printf("ERROR: rain sensor's adc value is incorrect! Possible defective pin or sensor\n");
        return SENSOR_ERROR; // Retorna um valor de erro
    }
    // O dispositivo funciona de modo inverso, ou seja, ao atingir 4096 no adc, o sensor está completamente seco, ao atingir em torno de 0 no adc, o sensor está completamente molhado
    return 100 - (rainAnalog * 100 / 4096); //Porcentagem = 100 - ((value*100)/4096)
}

uint8_t read_soil_humidity(void) {
    adc_select_input(ADC_CHANNEL_SOIL); // Seleção do ADC que será utilizado
    soilAnalog = adc_read();    // Leitura do ADC
    #ifdef DEBUG
    printf("adc SOIL: %d\n", soilAnalog);
    #endif
    if (soilAnalog < 0 || soilAnalog > 4096) {  //Problema com o pino do microcontrolador ou com o sensor
        printf("ERROR: soil moisture sensor's adc value is incorrect! Possible defective pin or sensor\n");
        return SENSOR_ERROR; // Retorna um valor de erro
    }
    // O dispositivo funciona de modo inverso, ou seja, ao atingir 4096 no adc, o sensor está completamente seco, ao atingir em torno de 0 no adc, o sensor está completamente molhado
    return 100 - (soilAnalog * 100 / 4096); //Porcentagem = 100 - ((value*100)/4096)
}
