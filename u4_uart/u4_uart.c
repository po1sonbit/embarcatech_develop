#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"

#define UART0       0
#define UART1       1

#define UART0_PORT  uart0
#define UART1_PORT  uart1
#define PIN_UART0_TX    0
#define PIN_UART0_RX    1
#define PIN_UART1_TX    8//4
#define PIN_UART1_RX    9//5

void on_uart1_rx();
void setup_uart(uint8_t uart_select);

int main() {
    stdio_init_all();

    setup_uart(UART0);
    setup_uart(UART1);

    irq_set_exclusive_handler(UART1_IRQ, on_uart1_rx);
    irq_set_enabled(UART1_IRQ, true);
    uart_set_irq_enables(UART1_PORT, true, false);

    while (true) {
        char message;
        uart_puts(UART0_PORT,"NEW MESSAGE$\n");

        // if(message == '\0') {
        //   printf("NEW MESSAGE\n");
        //   printf("Received message in UART0:%s", message);
        //   uart_putc(UART0_PORT, message);
        //   printf("\nSending message to UART1\n");
        //   message = '\0';
        // }
        sleep_ms(2000);
    }
}

void on_uart1_rx() {
  while(uart_is_readable(UART1_PORT)) {
    char message = uart_getc(UART1_PORT);
    printf("\nReceived message in UART1:%c\n", message);
    // if(message == '$') {
    //   printf("\nReceived message in UART1:%s\n", message);
    //   message = '\0';
    // }
  }
}

void setup_uart(uint8_t uart_select) {
  if(!uart_select) {
    uart_init(UART0_PORT, 115200);
    gpio_set_function(PIN_UART0_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART0_RX, GPIO_FUNC_UART);
    uart_set_format(UART0_PORT, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART0_PORT, false);
  }
  else {
    uart_init(UART1_PORT, 115200);
    gpio_set_function(PIN_UART1_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART1_RX, GPIO_FUNC_UART);
    uart_set_format(UART1_PORT, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(UART1_PORT, false);
  }
}