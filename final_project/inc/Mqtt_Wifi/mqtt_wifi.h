#ifndef MQTT_WIFI_H // include guard
#define MQTT_WIFI_H

#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#define HIGH                    1
#define LOW                     0

#define WIFI_SSID               "BANKSY-9!"
#define WIFI_PASSWORD           "bwh4ck34d0"
#define WIFI_ERROR              1
#define WIFI_OK                 0
#define WIFI_TIMEOUT            10000

#define MQTT_BROKER_HOSTNAME    "test.mosquitto.org"//"broker.hivemq.com"
#define MQTT_BROKER_PORT        1883

#define TOPIC_TEMP              "bitdog/temp"
#define TOPIC_SOIL              "bitdog/soil"
#define TOPIC_RAIN              "bitdog/rain"

extern mqtt_client_t *global_mqtt_client;

uint8_t start_Wifi(void);
void stop_Wifi(void);
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void start_MQTT(void);

#endif