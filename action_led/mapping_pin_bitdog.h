/*
 *  mapping_pin_bitdog.h
 *
 *  Created on: 15 aug 2025
 *      Author: Brendon Wesley (po1son.bit)
 */


/*
*   It is necessary add "mapping_pin_bitdog.h" in the root of your source code
*/

#ifndef MAPPING_PIN_BITDOG_H_
#define MAPPING_PIN_BITDOG_H_

#define PIN_LED_RED                 13
#define PIN_LED_BLUE                12
#define PIN_LED_GREEN               11

#define BUTTON_A_PIN                5
#define BUTTON_B_PIN                6

//OLED
#define I2C_OLED_PORT               i2c1    // Seleção de I2C
#define PIN_OLED_SDA                14      // GPIO SDA da I2C1
#define PIN_OLED_SCL                15      // GPIO SCL da I2C1
#define ADDRESS_OLED                0x3C    // Endereço do display Oled
#define WIDTH_OLED                  128     // Largura da tela efetiva do display Oled
#define HEIGHT_OLED                 64      // Altura da tela efetiva do display Oled

#endif // MAPPING_PIN_BITDOG_H_