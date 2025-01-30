#ifndef LORA_SX1280_H
#define LORA_SX1280_H

#include <stdio.h>
#include <pico/stdlib.h>

extern uint8_t humidity_percent;
extern uint8_t rain_percent;
extern float temperature;

uint8_t init_Lora(void);
void lora_rx(void);
#endif