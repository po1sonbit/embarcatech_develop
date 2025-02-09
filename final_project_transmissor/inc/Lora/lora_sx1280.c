#include "lora_sx1280.h"
#include <stdio.h>
#include "pico/stdlib.h"

#define SIZE_PACKET 4

uint8_t init_Lora(void) {
    printf("SX1280 will be started...\n");
    uint8_t state = SX1280_begin();

    if (state == true) {
        printf("Radio SX1280 initialized\n");
        SX1280_setupLoRa();
        printf("[SX1280] Ready to transmit\n");
    }
    else {
        while(true)
            printf("Failed to start\n");
    }
    return state;
}

void lora_tx(float temperature, uint8_t humidity_percent, uint8_t rain_percent) {
    uint8_t temp_int = (uint8_t)temperature;
    uint8_t temp_dec = (uint8_t)((temperature - (float)temp_int) * 100);
    SX1280_writeUint8(humidity_percent, 0);
    SX1280_writeUint8(rain_percent, 1);
    SX1280_writeUint8(temp_int, 2);
    SX1280_writeUint8(temp_dec, 3);

    printf("TXReady: %d\n", SX1280_isTXReady());

    SX1280_startWriteSXBuffer(0);
    SX1280_writeSXBuffer(SIZE_PACKET);
    SX1280_endWriteSXBuffer();
    SX1280_transmitSXBuffer(0, SIZE_PACKET, 0);
    printf("Lora Packet transmitted!!\n");
}