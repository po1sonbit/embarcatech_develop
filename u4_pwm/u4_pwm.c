#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

/*
FREQUENCY PWM -> 1kHz
Frequencia PWM = Clk PWM / Period + 1
Clk PWM = ClkSys / Divisor de clock pwm -> Clk PWM = 125Mhz/16 = 7,8125Mhz
Frequencia PWM = 1kHz
Period = (Clk PWM / Frequencia PWM) - 1 -> (7812500 Hz / 1000 Hz) - 1 = 7811,5
*/

#define FREQUENCY_LED_RED_HZ   1000 //1kHz
#define FREQUENCY_LED_BLUE_HZ  10000 //10kHz

#define DUTY_CYCLE_INITIAL  5 //em %
#define DUTY_CYCLE_MAX      100 //em %
#define DUTY_CYCLE_STEP     5 //em %
#define UPDATE_INTERVAL     2000

#define CLOCK_SYSTEM  125000000

#define PIN_LED_RED 13
#define PIN_LED_BLUE 12
#define PIN_LED_GREEN 11

#define PERIOD  7811.5 // periodo e nivel maximo do LED
#define DIVIDER_PWM 16.0
// #define LED_STEP  100 // passo e limite inferior
// #define PWM_REFRESH_LEVEL 15000

uint16_t led_level = 0; //nivel inicial LED
uint32_t period_blue = 0;
uint32_t period_red = 0;
volatile uint16_t duty_cycle = DUTY_CYCLE_INITIAL; // Duty cycle inicial

volatile uint32_t irq_count = 0;

void setup_pwm(uint8_t pin_led, uint32_t frequency, uint16_t duty_cycle);
void pwm_irq_red_handler(void);
void pwm_irq_blue_handler(void);
uint8_t change_dutycycle_red_callback(struct repeating_timer *t);
uint8_t change_dutycycle_blue_callback(struct repeating_timer *t);

int main() {
    stdio_init_all();

    setup_pwm(PIN_LED_RED, FREQUENCY_LED_RED_HZ, DUTY_CYCLE_INITIAL);

    //Como a questão diz apenas para colocar o led com uma frequència de PWM de 10Khz, o led azul apenas ficará aceso na frequência de PWM de 10Khz
    // setup_pwm(PIN_LED_BLUE, FREQUENCY_LED_BLUE_HZ, 100);

    // struct repeating_timer handle_timer;
    // struct repeating_timer handle_timer2;

    // add_alarm_in_ms(UPDATE_INTERVAL, change_dutycycle_callback, NULL, true);
    // add_repeating_timer_ms(UPDATE_INTERVAL, change_dutycycle_red_callback, NULL, &handle_timer);
    // add_repeating_timer_ms(UPDATE_INTERVAL, change_dutycycle_blue_callback, NULL, &handle_timer2);

    while (true) {
      sleep_ms(1);
    }
}

void setup_pwm(uint8_t pin_led, uint32_t frequency, uint16_t duty_cycle) {
  uint slice;

  gpio_set_function(pin_led, GPIO_FUNC_PWM);
  slice = pwm_gpio_to_slice_num(pin_led);
  pwm_set_clkdiv(slice, DIVIDER_PWM);
  if(pin_led == PIN_LED_RED) {
    period_red = (CLOCK_SYSTEM / (DIVIDER_PWM * frequency)) - 1;
    pwm_set_wrap(slice, period_red);
    pwm_set_gpio_level(pin_led, (period_red * duty_cycle) / 100); // 5% do period
    pwm_set_enabled(slice, true);

    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irq_red_handler);
    pwm_clear_irq(slice);
    pwm_set_irq_enabled(slice, true);
    irq_set_enabled(PWM_IRQ_WRAP, true);
  }
  else if(pin_led == PIN_LED_BLUE) {
    period_blue = (CLOCK_SYSTEM / (DIVIDER_PWM * frequency)) - 1;
    pwm_set_wrap(slice, period_blue);
    pwm_set_gpio_level(pin_led, (period_blue * duty_cycle) / 100); // 5% do period
    pwm_set_enabled(slice, true);

    pwm_set_gpio_level(PIN_LED_BLUE, (period_blue * duty_cycle) / 100);
  }
}

void pwm_irq_red_handler(void) {
  uint32_t slice = pwm_get_irq_status_mask();
  pwm_clear_irq(slice);

  uint32_t update_interval_hz = UPDATE_INTERVAL*1000;
  uint32_t update_cycle = update_interval_hz / (period_red + 1);
  update_cycle = UPDATE_INTERVAL*period_red;

  irq_count++;

  // if(count++ < UPDATE_INTERVAL){
  //   return;
  // }
  
  if(irq_count >= 7812500) {
    irq_count = 0;
    duty_cycle += DUTY_CYCLE_STEP;
    printf("2 segundos, duty cycle: %d\n", duty_cycle);
    if (duty_cycle > DUTY_CYCLE_MAX) {
      duty_cycle = DUTY_CYCLE_INITIAL;
      // printf("resetou para 5");
    }
    printf("level: %d\n", (period_red * duty_cycle) / 100);
    pwm_set_gpio_level(PIN_LED_RED, (period_red * duty_cycle) / 100); // aumento do duty cycle em porcentagem
  }
}

uint8_t change_dutycycle_red_callback(struct repeating_timer *t) {
  // static uint32_t count = 0;
  // count = 0;
  printf("2 segundos.\n");
  duty_cycle += DUTY_CYCLE_STEP;
  if (duty_cycle > DUTY_CYCLE_MAX) {
    duty_cycle = DUTY_CYCLE_INITIAL;
  }
  printf("duty_cycle: %d\n", duty_cycle);
  printf("level: %d\n", (period_red * duty_cycle) / 100);
  pwm_set_gpio_level(PIN_LED_RED, (period_red * duty_cycle) / 100);
  return true; // Retorna 0 para não repetir o alarme.
}
