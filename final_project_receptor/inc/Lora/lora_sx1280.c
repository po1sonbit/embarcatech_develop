#include "lora_sx1280.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "inc/SX1280/SX1280.h"

#define SIZE_PACKET 4

int16_t rssi = 0;
uint8_t buffer_received[SIZE_PACKET];

uint8_t humidity_percent = 0;
uint8_t rain_percent = 0;
float temperature = 0;

uint8_t init_Lora(void) {
    printf("SX1280 will be started...\n");
    uint8_t state = SX1280_begin();

    if (state == true) {
        printf("Radio SX1280 initialized\n");
        SX1280_setupLoRa();
        printf("[SX1280] Ready to receive\n");
    }
    else {
        while(true) {
            printf("Failed to start\n");
        }
    }
    return state;
}

uint8_t lora_rx(void) {
    uint8_t temp_int = 0;
    uint8_t temp_dec = 0;
    if (SX1280_checkIrqs()) {
        gpio_init(PIN_LED_BLUE);
        gpio_set_dir(PIN_LED_BLUE, GPIO_OUT);
        gpio_pull_up(PIN_LED_BLUE);

        gpio_put(PIN_LED_BLUE, 1);
        sleep_ms(500);
        gpio_put(PIN_LED_BLUE, 0);
        
        SX1280_receiveSXBuffer(0, 0);
        rssi = SX1280_readPacketRSSI();

        SX1280_startReadSXBuffer(0);
        SX1280_readSXBuffer(SIZE_PACKET);
        SX1280_endReadSXBuffer();

        SX1280_getSXBuffer(buffer_received, SIZE_PACKET);

        humidity_percent = SX1280_readUint8(0);
        printf("Soil Humidity percentage: %d %%\n", humidity_percent);
        rain_percent = SX1280_readUint8(1);
        printf("Rain percentage: %d %%\n", rain_percent);
        temp_int = SX1280_readUint8(2);
        temp_dec = SX1280_readUint8(3);
        temperature = (float)temp_int + (float)(temp_dec/100.0);
        printf("Temperature: %f C\n", temperature);

        printf("Message received: \n");
        for(int i = 0; i < SIZE_PACKET; i++) {
            printf("%d ", buffer_received[i]);
        }
        printf("\n");
        printf("RSSI: %d\n", rssi);
        return PACK_RECEIVED;
    }
    return PACK_NOT_RECEIVED;
}