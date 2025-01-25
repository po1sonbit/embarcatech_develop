#include "lora_sx1280.h"
#include <stdio.h>
// #include <iostream>
#include "pico/stdlib.h"
#include "inc/SX1280/SX1280.h"

int16_t rssi = 0;
uint8_t* buffer_received;

SX1280 sx1280;

void init_Lora(void) {
    printf("SX1280 will be started...\n");
    uint8_t state = sx1280.begin();

    if (state == true) {
        printf("Radio SX1280 initialized\n");
        sx1280.setupLoRa();
        printf("[SX1280] Starting to listen ... \n");
    }
    else {
        printf("Failed to start\n");
    }
}

void lora_rx(void) {
    if (sx1280.checkIrqs()) {
        sx1280.receiveSXBuffer(0, 0);
        rssi = sx1280.readPacketRSSI();

        sx1280.startReadSXBuffer(0);
        sx1280.readSXBuffer(8);
        sx1280.endReadSXBuffer();

        sx1280.getSXBuffer(buffer_received, 8);

        // pressure = sx1280.readUint16(0);
        // asx1280itude = sx1280.readInt16(2);
        // temperature = sx1280.readFloat(4);

        printf("BUFFER RECEBIDO");
        for(int i = 0; i < 8; i++) {
            printf("%x ", buffer_received[i]);
        }
        printf("\n");

        // cout << "pressure: " << pressure << " asx1280itude: " << asx1280itude << " temperature: " << temperature << endl;
        // cout << "rssi : " << rssi << endl;
    }
}