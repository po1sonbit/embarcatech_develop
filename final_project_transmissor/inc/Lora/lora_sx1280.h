#ifndef lora_sx1280_H
#define lora_sx1280_H

#include <stdio.h>
#include <pico/stdlib.h>
#include "inc/SX1280/SX1280.h"

uint8_t init_Lora(void);
void lora_tx(float temperature, uint8_t humidity_percent, uint8_t rain_percent);
#endif