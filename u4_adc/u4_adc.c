#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

/*
Celsius to Fahrenheit
C/5 = F-32/9 -> F = 1,8C + 32
*/

/* DESCOMENTE A MEDIDA DE TEMPERATURA DESEJADA */
// #define MODE_CELSIUS
#define MODE_FAHRENHEIT
//////////////////////////////////////////////

#define ADC_CHANNEL     4
#define DELAY_READINGS  1000
#define CELSIUS         0
#define FAHRENHEIT      1

void adc_temp_init(void);
float adc_to_temperature(uint16_t adc_value, uint8_t temp_meas);
void routine_temp(void);

int main() {
    stdio_init_all();
    adc_temp_init();

    while (true) {
      routine_temp();
    }
}

void adc_temp_init(void) {
  adc_init();

  adc_set_temp_sensor_enabled(true);
  adc_select_input(ADC_CHANNEL);
}

float adc_to_temperature(uint16_t adc_value, uint8_t temp_meas) {
    const float conversion_factor = 3.3f / (1 << 12);
    float voltage = adc_value * conversion_factor;
    float temperature_celsius = 27.0f - (voltage - 0.706f) / 0.001721f;
    float temperature_fahrenheit = (1.8*temperature_celsius) + 32;
    if(!temp_meas) 
      return temperature_celsius;
    else
      return temperature_fahrenheit;
}

void routine_temp(void) {
  uint16_t adc_value = adc_read();
  printf("adc_value: %d\n", adc_value);

  #ifdef MODE_CELSIUS
    float temperature = adc_to_temperature(adc_value, CELSIUS);
    printf("Temperature: %.2f Celsius\n", temperature);
  #else
    float temperature = adc_to_temperature(adc_value, FAHRENHEIT);
    printf("Temperature: %.2f F\n", temperature);
  #endif
  sleep_ms(DELAY_READINGS);
}
