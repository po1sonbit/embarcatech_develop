#ifndef MQTT_WIFI_H // include guard
#define MQTT_WIFI_H

#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#define HIGH                    1
#define LOW                     0
#define PIN_LED_RED                 13
#define PIN_LED_BLUE                12
#define PIN_LED_GREEN               11

#define WIFI_SSID               "RSB 2.4g"
#define WIFI_PASSWORD           "23171621"
#define WIFI_ERROR              1
#define WIFI_OK                 0
#define WIFI_TIMEOUT            10000

#define MQTT_BROKER_HOSTNAME    "broker.hivemq.com"//"broker.hivemq.com"
#define MQTT_BROKER_PORT        1883//1883

#define TOPIC_TEST              "bitdog/test"
#define TOPIC_TEMP              "bitdog/temp"
#define TOPIC_PRESS             "bitdog/press"
#define TOPIC_X_AXIS            "bitdog/xaxis"

#define TOPIC_SUB_LED           "bitdog/sub/led"

extern mqtt_client_t *global_mqtt_client;
extern uint8_t led_blue_status;

uint8_t start_Wifi(void);
void stop_Wifi(void);
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_sub_request_cb(void *arg, err_t err);
// void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void start_MQTT(void);

#endif