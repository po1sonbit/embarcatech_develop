#include "Server_homework2.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h" // Para acessar netif_default e IP

#include "bmp280.h"

/*
ABRE UM WEBSERVER E ENVIA STATUS DE UM BOTÃO
COMUNICA VIA I2C AO BMP280 E ENVIA OS DADOS
RECEBE POSIÇÃO DO JOYSTICK
*/

uint16_t raw_x = 0;
uint16_t raw_y = 0;
int32_t raw_temperature;
int32_t raw_pressure;

struct bmp280_calib_param params;

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

int main()
{
    stdio_init_all();

    adc_init();

    adc_gpio_init(ADC_PIN_JOYSTICK_X);
    adc_gpio_init(ADC_PIN_JOYSTICK_Y);
    
    i2c_init(I2C_PORT, 100*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    bmp280_init();

    bmp280_get_calib_params(&params);

    while (cyw43_arch_init())
    {
        printf("Falha ao inicializar Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    cyw43_arch_gpio_put(LED_PIN, 0);
    cyw43_arch_enable_sta_mode();

    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    printf("Conectado ao Wi-Fi\n");

    if (netif_default)
    {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    // Configura o servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server)
    {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    printf("Servidor ouvindo na porta 80\n");

    while (true)
    {
        cyw43_arch_poll();
    }
    cyw43_arch_deinit();
}

// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf("Requisição HTTP recebida!\n");
    if (!p) {
        printf("Conexão fechada pelo cliente.\n");
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    bmp280_read_raw(&raw_temperature, &raw_pressure);
    int32_t temperature = bmp280_convert_temp(raw_temperature, &params);
    float temperature_f = temperature / 100.0;
    int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temperature, &params);
    float pressure_f = pressure / 1000.0;
    printf("Pressure = %.3f kPa\n", pressure_f);
    printf("Temp. = %.2f C\n", temperature_f);

    // Lê joystick
    adc_select_input(ADC_CHANNEL_JOYSTICK_X);
    sleep_us(2);
    raw_x = adc_read();
    adc_select_input(ADC_CHANNEL_JOYSTICK_Y);
    sleep_us(2);
    raw_y = adc_read();

    // Lê botão
    bool button_state = (gpio_get(BUTTON_A) == 0);

    // Lógica da Rosa dos Ventos
    const char *direcao = "Centro";
    
    const char *compass_art =
        "      N      \n"
        "   NW + NE   \n"
        "      |      \n"
        "W --- O --- E\n"
        "      |      \n"
        "   SW + SE   \n"
        "      S      \n";
    
    // Mapeamento dos valores do joystick para a direção e o desenho
    if (raw_y > 3000) {
        if (raw_x < 1000) {
            direcao = "Noroeste";
            compass_art =
                "      N      \n"
                "   -> NW <-  \n"
                "      |      \n"
                "W --- O --- E\n"
                "      |      \n"
                "   SW + SE   \n"
                "      S      \n";
        } else if (raw_x > 3000) {
            direcao = "Nordeste";
            compass_art =
                "      N      \n"
                "   -> NE <-  \n"
                "      |      \n"
                "W --- O --- E\n"
                "      |      \n"
                "   SW + SE   \n"
                "      S      \n";
        } else {
            direcao = "Norte";
            compass_art =
                "   -> N <-   \n"
                "   NW + NE   \n"
                "      |      \n"
                "W --- O --- E\n"
                "      |      \n"
                "   SW + SE   \n"
                "      S      \n";
        }
    } else if (raw_y < 1000) {
        if (raw_x < 1000) {
            direcao = "Sudoeste";
            compass_art =
                "      N      \n"
                "   NW + NE   \n"
                "      |      \n"
                "W --- O --- E\n"
                "      |      \n"
                "   -> SW <-  \n"
                "      S      \n";
        } else if (raw_x > 3000) {
            direcao = "Sudeste";
            compass_art =
                "      N      \n"
                "   NW + NE   \n"
                "      |      \n"
                "W --- O --- E\n"
                "      |      \n"
                "   -> SE <-  \n"
                "      S      \n";
        } else {
            direcao = "Sul";
            compass_art =
                "      N      \n"
                "   NW + NE   \n"
                "      |      \n"
                "W --- O --- E\n"
                "      |      \n"
                "   SW + SE   \n"
                "   -> S <-   \n";
        }
    } else {
        if (raw_x < 1000) {
            direcao = "Oeste";
            compass_art =
                "      N      \n"
                "   NW + NE   \n"
                "      |      \n"
                "-> W <- O --- E\n"
                "      |      \n"
                "   SW + SE   \n"
                "      S      \n";
        } else if (raw_x > 3000) {
            direcao = "Leste";
            compass_art =
                "      N      \n"
                "   NW + NE   \n"
                "      |      \n"
                "W --- O -> E <-\n"
                "      |      \n"
                "   SW + SE   \n"
                "      S      \n";
        }
    }
    
    char response[2048];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "\r\n"
             "<!DOCTYPE html><html><head>"
             "<meta charset='UTF-8'>"
             "<meta http-equiv='refresh' content='1'>" 
             "<title>Homework2</title>"
             "<style>"
             "body { font-family: Arial, sans-serif; text-align: center; background: #f0f0f0; }"
             ".card { background: white; margin: 50px auto; padding: 30px; border-radius: 15px;"
             "box-shadow: 0 4px 12px rgba(0,0,0,0.2); width: 400px; }"
             "h1 { font-size: 28px; color: #333; }"
             "p { font-size: 22px; margin: 15px 0; }"
             ".pressed { color: green; font-weight: bold; }"
             ".released { color: red; font-weight: bold; }"
             "#direcao { font-size: 30px; font-weight: bold; margin-top: 20px; color: #007bff; }"
             "</style>"
             "</head><body>"
             "<div class='card'>"
             "<h1>Rosa dos Ventos, Status do Botão, BMP280</h1>"
             "<p>Eixo X: %u</p>"
             "<p>Eixo Y: %u</p>"
             "<p>Temperatura: %.2f &deg;C</p>\n"
             "<p>Pressão: %.3f kPA</p>\n"
             "<p>Botão: <span class='%s'>%s</span></p>"
             "<div id='direcao'>Direção: %s</div>" // Exibindo apenas a direção em texto
             "<pre>%s</pre>"
             "</div>"
             "</body></html>",
             raw_x, raw_y,
             temperature_f, pressure_f,
             button_state ? "pressed" : "released",
             button_state ? "PRESSIONADO" : "SOLTO",
             direcao, 
             compass_art);

    tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    // tcp_close(tpcb);
    // tcp_recv(tpcb, NULL);

    free(request);
    pbuf_free(p);

    return ERR_OK;
}

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}