#include "mqtt_wifi.h"

mqtt_client_t *global_mqtt_client = NULL;

uint8_t start_Wifi(void) {
    if (cyw43_arch_init()) {
        printf("Failed to start WiFi\n");
        return WIFI_ERROR;
    }

    cyw43_arch_enable_sta_mode();
    printf("Trying to connect WiFi in STA...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT)) {
        printf("Timeout: Failed to connect WiFi\n");
        return WIFI_ERROR;
    }
    else {
        printf("WiFi Connected!\n");
        gpio_put(PIN_LED_BLUE, HIGH);
        sleep_ms(500);
        gpio_put(PIN_LED_BLUE, LOW);

    }
    return WIFI_OK;
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Connection MQTT Success!\n");
        gpio_put(PIN_LED_GREEN, HIGH);
        sleep_ms(500);
        gpio_put(PIN_LED_GREEN, LOW);
    } else {
        printf("Connection MQTT Failure: %d\n", status);
    }
}

void mqtt_dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr) {
        printf("MQTT Broker IP: %s\n", ipaddr_ntoa(ipaddr));

        struct mqtt_connect_client_info_t ci = {
            .client_id = "bitdoglab_client",
            .client_user = NULL,
            .client_pass = NULL,
            .keep_alive = 60
        };

        err_t err = mqtt_client_connect(global_mqtt_client, ipaddr, MQTT_BROKER_PORT, NULL, NULL, &ci);
        if (err != ERR_OK) {
            printf("MQTT connect failed: %d\n", err);
        }
    } else {
        printf("DNS resolve failed for %s\n", name);
    }
}

void start_MQTT(void) {
    global_mqtt_client = mqtt_client_new();
    if (!global_mqtt_client) {
        printf("Failed to create MQTT client\n");
        return;
    }

    ip_addr_t mqtt_ip_address;
    err_t err = dns_gethostbyname(MQTT_BROKER_HOSTNAME, &mqtt_ip_address, mqtt_dns_found_cb, NULL);

    if (err == ERR_OK) {
        mqtt_dns_found_cb(MQTT_BROKER_HOSTNAME, &mqtt_ip_address, NULL);
    } else if (err != ERR_INPROGRESS) {
        printf("DNS request failed: %d\n", err);
    }
}