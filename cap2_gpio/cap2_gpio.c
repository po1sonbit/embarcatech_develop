#include <stdio.h>
#include "pico/stdlib.h"

#define HIGH    1
#define LOW     0

//Descomente apenas a opção de codigo de interesse
/*-----------------------------------------------------*/
// #define EXAMPLE1     //LED e botao
// #define EXAMPLE2     //LED RGB e dois botoes
#define EXAMPLE3     //Teclado matricial
/*-----------------------------------------------------*/

#ifdef EXAMPLE1
#define LED_PIN         11
#define BUTTON_PIN      5

#elif  defined(EXAMPLE2)
#define BUTTON_A_PIN    5
#define BUTTON_B_PIN    6
#define RED_LED_PIN     13
#define BLUE_LED_PIN    12
#define GREEN_LED_PIN   11

uint8_t led_blue_status = false;
uint8_t led_red_status = false;
uint8_t led_green_status = false;

void verify_buttons(uint8_t _buttonA, uint8_t _buttonB, uint8_t* _led_status1, uint8_t* _led_status2, uint8_t* _led_status3);
void control_Led(uint8_t _led_pin, uint8_t _led_status);

#elif defined(EXAMPLE3)
const uint8_t COL_PINS[] = {1, 2, 3, 4};
const uint8_t ROW_PINS[] = {5, 6, 7, 8};
const char KEY_MAP[4][4] = {
    {'D', '#', '0', '*'},
    {'C', '9', '8', '7'},
    {'B', '6', '5', '4'},
    {'A', '3', '2', '1'}
};
char read_keypad(uint8_t* colums, uint8_t* rows);

char key_pressed = {0};
#endif

int main() {
    stdio_init_all();
    #ifdef  EXAMPLE1
        gpio_init(LED_PIN);
        gpio_init(BUTTON_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        gpio_set_dir(BUTTON_PIN, GPIO_IN);
        gpio_pull_up(BUTTON_PIN);

        while(true) {
            while(!gpio_get(BUTTON_PIN)) {
                gpio_put(LED_PIN, HIGH);
            }
            gpio_put(LED_PIN, LOW);
        }
    #elif defined(EXAMPLE2)
        gpio_init(RED_LED_PIN);
        gpio_init(BLUE_LED_PIN);
        gpio_init(GREEN_LED_PIN);
        gpio_init(BUTTON_A_PIN);
        gpio_init(BUTTON_B_PIN);

        gpio_set_dir(RED_LED_PIN, GPIO_OUT);
        gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
        gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
        gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
        gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
        gpio_pull_up(BUTTON_A_PIN);
        gpio_pull_up(BUTTON_B_PIN);

        gpio_put(GREEN_LED_PIN, LOW);
        gpio_put(RED_LED_PIN, LOW);
        gpio_put(BLUE_LED_PIN, LOW);
        while(true) {
            verify_buttons(BUTTON_A_PIN, BUTTON_B_PIN, &led_red_status, &led_blue_status, &led_green_status);
            
            control_Led(RED_LED_PIN, led_red_status);
            control_Led(BLUE_LED_PIN, led_blue_status);
            control_Led(GREEN_LED_PIN, led_green_status);
        }
    #elif defined(EXAMPLE3)
    //falta iniciar os pinos
        key_pressed = read_keypad(COL_PINS, ROW_PINS);
        printf("Botao pressionado %c\n", key_pressed);
    #endif
}

char read_keypad(uint8_t* colums, uint8_t* rows) {
    for(int i = 0; i < 4; i++) {
        gpio_put(rows[i], LOW);
        uint8_t result = 0;
        for(int j = 0; j < 4; j++) {
            result |= gpio_get(colums[j]);
        }
        if(result == 0) {
            char key = KEY_MAP[i][__builtin_ctz(result)];
            gpio_put(rows[i], HIGH);
            return key;
        }
        gpio_put(rows[i], HIGH);
    }
    return 0;
}

void verify_buttons(uint8_t _buttonA, uint8_t _buttonB, uint8_t* _led_status1, uint8_t* _led_status2, uint8_t* _led_status3) {
    if((!gpio_get(_buttonA)) && (gpio_get(_buttonB))) {
        *_led_status1 = false;
        *_led_status2 = true;
        *_led_status3 = false;
    }
    else if((!gpio_get(_buttonB)) && (gpio_get(_buttonA))) {
        *_led_status1 = true;
        *_led_status2 = false;
        *_led_status3 = false;
    }
    else if((!gpio_get(_buttonA)) && (!gpio_get(_buttonB))) {
        *_led_status1 = false;
        *_led_status2 = false;
        *_led_status3 = true;
    }
    else {
        *_led_status1 = false;
        *_led_status2 = false;
        *_led_status3 = false;
    }
}

void control_Led(uint8_t _led_pin, uint8_t _led_status) {
    if(_led_status && (!gpio_get(_led_pin))){
        gpio_put(_led_pin, HIGH);
        printf("Led ON\n");
    }
    if(!_led_status && (gpio_get(_led_pin))) {
        gpio_put(_led_pin, LOW);
        printf("Led OFF\n");
    }
}
