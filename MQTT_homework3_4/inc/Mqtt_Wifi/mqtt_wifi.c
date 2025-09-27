#include "mqtt_wifi.h"

#include "hardware/watchdog.h"

mqtt_client_t *global_mqtt_client = NULL;

static int inpub_id;
uint8_t led_blue_status = 0;

static void stop_MQTT(void);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

uint8_t start_Wifi(void) {
    while(cyw43_arch_init()) {
        printf("Failed to start WiFi, trying connect again...\n");
    }

    cyw43_arch_enable_sta_mode();
    printf("Trying to connect WiFi in STA...\n");

    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, WIFI_TIMEOUT)) {
        watchdog_update();
        printf("Timeout: Failed to connect WiFi, trying again...\n");
    }
    printf("WiFi Connected!\n");
    gpio_put(PIN_LED_GREEN, HIGH);
    gpio_put(PIN_LED_RED, HIGH);
    gpio_put(PIN_LED_BLUE, HIGH);
    sleep_ms(500);
    gpio_put(PIN_LED_GREEN, LOW);
    gpio_put(PIN_LED_RED, LOW);
    gpio_put(PIN_LED_BLUE, LOW);

    return WIFI_OK;
}

void stop_Wifi(void) {
    stop_MQTT();
    printf("Disabling WiFi...\n");
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    netif_set_down(netif_default);
    cyw43_arch_deinit();  // Desativa completamente o Wi-Fi
    sleep_ms(500);
    printf("WiFi Disabled!\n");
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Connection MQTT Success!\n");
        gpio_put(PIN_LED_GREEN, HIGH);
        sleep_ms(500);
        gpio_put(PIN_LED_GREEN, LOW);
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
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
        watchdog_update();
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
    err_t err = dns_gethostbyname(MQTT_BROKER_HOSTNAME, &mqtt_ip_address, mqtt_dns_found_cb, NULL);

    if (err == ERR_OK) {
        mqtt_dns_found_cb(MQTT_BROKER_HOSTNAME, &mqtt_ip_address, NULL);
    } else if (err != ERR_INPROGRESS) {
        printf("DNS request failed: %d\n", err);
    }
}

static void stop_MQTT(void) {
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

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    printf( "TOPICO SUB: %s, TAMANHO DA MENSAGEM RECEBIDA: %u, inpub_id: %d\n\r", topic, (unsigned int)tot_len, inpub_id);
  
    /* Decode topic string into a user defined reference */
    if(strcmp(topic, TOPIC_SUB_LED) == 0) {
        inpub_id = 0;
    }
    else if(topic[0] == 'A'){
        /* All topics starting with 'A' might be handled at the same way */
        inpub_id = 1;
    }
    else{
        /* For all other topics */
        inpub_id = 2;
    }
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
  printf("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);

  if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
       See MQTT_VAR_HEADER_BUFFER_LEN)  */

    /* Call function or do action depending on reference, in this case inpub_id */
    if(inpub_id == 0) {
      /* Don't trust the publisher, check zero termination */
    //   if(data[len-1] == 0) {
        printf("mqtt_incoming_data_cb: %.5s\n", (const char *)data);
        if(data[0] == '1') {
            gpio_put(PIN_LED_BLUE, HIGH);
            led_blue_status = 1;
        }
        if(data[0] == '0') {
            gpio_put(PIN_LED_BLUE, LOW);
            led_blue_status = 0;
        }
    //   }
    } else if(inpub_id == 1) {
      /* Call an 'A' function... */
    } else {
      printf("mqtt_incoming_data_cb: Ignoring payload...\n");
    }
  } else {
    /* Handle fragmented payload, store in buffer, write to file or whatever */
  }
}

void mqtt_sub_request_cb(void *arg, err_t err) {
    printf("Subscribe result: %d\n", err);
}