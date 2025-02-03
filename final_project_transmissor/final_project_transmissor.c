#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/timer.h"
#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"
#include <dht.h>

#define SENSOR_ERROR            255
#define DEBUG
#define TIMER_FREQUENCY_MS      3000

#define PIN_RAIN                26
#define PIN_SOIL                27
#define PIN_DHT                 15

#define PIO                     pio0
#define INTERNAL_PULL_UP_DHT    true

#define ADC_CHANNEL_RAIN        0
#define ADC_CHANNEL_SOIL        2

static const dht_model_t DHT_MODEL = DHT11;

int soilAnalog = 0;
int rainAnalog = 0;
uint8_t percent_rain = 0;
uint8_t percent_humidity_soil = 0;
float temperature = 0;
float humidity = 0;

dht_t DHT;

uint8_t read_rain(void);
uint8_t read_soil_humidity(void);

uint8_t read_sensors_transmit(struct repeating_timer *t);

int main()
{
    stdio_init_all();

    adc_init();
    adc_gpio_init(PIN_RAIN);
    adc_gpio_init(PIN_SOIL);

    dht_init(&DHT, DHT_MODEL, PIO, PIN_DHT, INTERNAL_PULL_UP_DHT);

    init_Lora();

    struct repeating_timer handle_timer;
    add_repeating_timer_ms(TIMER_FREQUENCY_MS, read_sensors_transmit, NULL, &handle_timer);
    

    while (true) {
        sleep_ms(1);
    }
}

uint8_t read_sensors_transmit(struct repeating_timer *t) {
    percent_humidity_soil = read_soil_humidity();
    if(percent_humidity_soil != SENSOR_ERROR) {
        printf("Humidity Soil percentage: %d %%\n", percent_humidity_soil);
    }

    percent_rain = read_rain();
    if(percent_rain != SENSOR_ERROR) {
        printf("Rain percentage: %d %%\n", percent_rain);
    }
    
    dht_start_measurement(&DHT);
    dht_result_t measure = dht_finish_measurement_blocking(&DHT, &humidity, &temperature);
    if (measure == DHT_RESULT_OK) {
        printf("%.1f C\n", temperature);
    }
    else if (measure == DHT_RESULT_TIMEOUT) {
        temperature = SENSOR_ERROR;
        printf("DHT sensor not responding. Please check your wiring.\n");
    } 
    else {
        assert(measure == DHT_RESULT_BAD_CHECKSUM);
        temperature = SENSOR_ERROR;
        printf("Bad checksum\n");
    }
    lora_tx(temperature, percent_humidity_soil, percent_rain);
    return 1;
}

uint8_t read_rain(void) {
    adc_select_input(ADC_CHANNEL_RAIN);
    rainAnalog = adc_read();
    #ifdef DEBUG
    printf("adc RAIN: %d\n", rainAnalog);
    #endif
    if (rainAnalog < 0 || rainAnalog > 4096) {
        printf("ERROR: rain sensor's adc value is incorrect!\n");
        return SENSOR_ERROR; // Retorna um valor inválido
    }
    return 100 - (rainAnalog * 100 / 4096); //Porcentagem = 100 - ((value*100)/4096)
}

uint8_t read_soil_humidity(void) {
    adc_select_input(ADC_CHANNEL_SOIL);
    soilAnalog = adc_read();
    #ifdef DEBUG
    printf("adc SOIL: %d\n", soilAnalog);
    #endif
    if (soilAnalog < 0 || soilAnalog > 4096) {
        printf("ERROR: soil moisture sensor's adc value is incorrect!\n");
        return SENSOR_ERROR; // Retorna um valor inválido
    }
    return 100 - (soilAnalog * 100 / 4096); //Porcentagem = 100 - ((value*100)/4096)
}
