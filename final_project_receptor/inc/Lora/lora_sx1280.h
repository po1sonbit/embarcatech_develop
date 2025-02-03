#ifndef LORA_SX1280_H
#define LORA_SX1280_H

#include <stdio.h>
#include <pico/stdlib.h>

#define PIN_LED_RED             13
#define PIN_LED_BLUE            12
#define PIN_LED_GREEN           11

#define PACK_RECEIVED           0
#define PACK_NOT_RECEIVED       1

extern uint8_t humidity_percent;
extern uint8_t rain_percent;
extern float temperature;

uint8_t init_Lora(void);
uint8_t lora_rx(void);
#endif