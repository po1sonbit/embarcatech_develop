#include "mqtt_wifi.h"

// #include "inc/SX1280/Settings.h"

mqtt_client_t *global_mqtt_client = NULL;

void stop_MQTT(void);

uint8_t start_Wifi(void) {
    gpio_init(PIN_LED_BLUE);
    gpio_set_dir(PIN_LED_BLUE, GPIO_OUT);
    gpio_pull_up(PIN_LED_BLUE);

    // gpio_put(NSS, 1);

    // if (cyw43_arch_init()) {
    //     printf("Failed to start WiFi\n");
    //     return WIFI_ERROR;
    // }
    while(cyw43_arch_init()) {
        printf("Failed to start WiFi, trying connect again...\n");
    }

    cyw43_arch_enable_sta_mode();
    printf("Trying to connect WiFi in STA...\n");

    // if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT)) {
    //     printf("Timeout: Failed to connect WiFi\n");
    //     return WIFI_ERROR;
    // }
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT)) {
        printf("Timeout: Failed to connect WiFi, trying again...\n");
    }
    // else {
        printf("WiFi Connected!\n");
        gpio_put(PIN_LED_BLUE, HIGH);
        sleep_ms(500);
        gpio_put(PIN_LED_BLUE, LOW);

    // }
    return WIFI_OK;
}

void stop_Wifi(void) {
    stop_MQTT();
    printf("Disabling WiFi...\n");
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    netif_set_down(netif_default);
    // cyw43_arch_disable();
    cyw43_arch_deinit();  // Desativa completamente o Wi-Fi
    // gpio_put(NSS, 0);
    sleep_ms(500);
    printf("WiFi Disabled!\n");
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    gpio_init(PIN_LED_GREEN);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);
    gpio_pull_up(PIN_LED_GREEN);
    printf("entrou aqui\n");

    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Connection MQTT Success!\n");
        // gpio_put(PIN_LED_GREEN, HIGH);
        // sleep_ms(500);
        // gpio_put(PIN_LED_GREEN, LOW);
    } else {
        printf("Connection MQTT Failure: %d\n", status);
    }
}

void mqtt_dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr) {
        printf("MQTT Broker IP: %s\n", ipaddr_ntoa(ipaddr));

        sleep_ms(500);

        struct mqtt_connect_client_info_t ci = {
            .client_id = "bitdoglab_client",
            .client_user = NULL,
            .client_pass = NULL,
            .keep_alive = 60
        };

        // while(mqtt_client_connect(global_mqtt_client, ipaddr, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci) != ERR_OK) {
        //     printf("MQTT connect failed, trying again...\n");
        // }
        cyw43_arch_lwip_begin();
        err_t err = mqtt_client_connect(global_mqtt_client, ipaddr, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
        cyw43_arch_lwip_end();
        sleep_ms(4000);
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
    // IP4_ADDR(&mqtt_ip_address, 91, 121, 93, 94);
    err_t err = dns_gethostbyname(MQTT_BROKER_HOSTNAME, &mqtt_ip_address, mqtt_dns_found_cb, NULL);

    if (err == ERR_OK) {
        mqtt_dns_found_cb(MQTT_BROKER_HOSTNAME, &mqtt_ip_address, NULL);
    } else if (err != ERR_INPROGRESS) {
        printf("DNS request failed: %d\n", err);
    }
}

void stop_MQTT(void) {
    if (global_mqtt_client) {
        printf("Disconnecting MQTT...\n");
        mqtt_disconnect(global_mqtt_client);
        mem_free(global_mqtt_client);
        global_mqtt_client = NULL;

        printf("MQTT Disconnected!\n");
    } else {
        printf("MQTT Client not initialized!\n");
    }
}