#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "inc/Mqtt_Wifi/mqtt_wifi.h"

#include "inc/SX1280/SX1280.h"
#include "inc/Lora/lora_sx1280.h"

int count = 0;

void stop_spi_lora();

int main()
{
    stdio_init_all();

    // // Initialise the Wi-Fi chip
    // if (cyw43_arch_init()) {
    //     printf("Wi-Fi init failed\n");
    //     return -1;
    // }

    // // Enable wifi station
    // cyw43_arch_enable_sta_mode();

    // printf("Connecting to Wi-Fi...\n");
    // while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    //     printf("Timeout: Failed to connect WiFi, trying again...\n");
    //     // return 1;
    // }
    // // } else {
    //     printf("Connected.\n");
    //     // Read the ip address in a human readable way
    //     uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
    //     printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    // // }

    // start_Wifi();

    // start_MQTT();
    // sleep_ms(10000);

    // cyw43_arch_lwip_begin();
    // mqtt_publish(global_mqtt_client, TOPIC_TEST, "testemqtt", 9, 1, 0, NULL, NULL);
    // cyw43_arch_lwip_end();
    sleep_ms(3000);
    while (true) {
        init_Lora();
        // start_Wifi();
        printf("Hello, world!\n");
        sleep_ms(5000);
        stop_spi_lora();
        start_Wifi();
        sleep_ms(5000);
        // cyw43_arch_lwip_begin();
        // mqtt_publish(global_mqtt_client, TOPIC_TEST, "testemqtt", 9, 1, 0, NULL, NULL);
        // cyw43_arch_lwip_end();
        // stop_MQTT();
        stop_Wifi();
        init_Lora();
        while(1) {
            count++;
            lora_rx();
            printf("Waiting packets...\n");
            sleep_ms(100);
            if(count > 100) {
                count = 0;
                // stop_spi_lora();
                start_Wifi();
                sleep_ms(5000);
                stop_Wifi();
                // init_Lora();
                while(1) {
                    lora_rx();
                    printf("Waiting packets...\n");
                    sleep_ms(100);
                }
            }
        }
    }
}

void stop_spi_lora() {
    spi_deinit(SPI_PORT);
    printf("PIN_MISO:%d\nPIN_MOSI:%d\nPIN_SCK:%d\nNSS:%d\nNRESET:%d\nRFBUSY:%d\nDIO1:%d\n", gpio_get(PIN_MISO),  gpio_get(PIN_MOSI),
                                                                                            gpio_get(PIN_SCK), gpio_get(NSS), gpio_get(NRESET),
                                                                                            gpio_get(RFBUSY), gpio_get(DIO1));
    printf("STOP LORA\n");
}
