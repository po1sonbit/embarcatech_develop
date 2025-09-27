#define ADC_PIN_JOYSTICK_X              27
#define ADC_PIN_JOYSTICK_Y              26

#define ADC_CHANNEL_JOYSTICK_X          1
#define ADC_CHANNEL_JOYSTICK_Y          0

// #define I2C_PORT_SENSOR                 i2c0
// #define I2C_SDA_SENSOR                  0
// #define I2C_SCL_SENSOR                  1

#define I2C_PORT_OLED                   i2c1
#define I2C_SDA_OLED                    14
#define I2C_SCL_OLED                    15
#define ADDRESS_OLED                    0x3C    // Endereço do display Oled
#define WIDTH_OLED                      128     // Largura da tela efetiva do display Oled
#define HEIGHT_OLED                     64      // Altura da tela efetiva do display Oled

#define TIMER_FREQUENCY_MS          30000   // Tempo para chamar rotina de envio pro MQTT

#define MULT_FREQUENCY_KHZ          1000    // Multiplicador de frequência
#define I2C_FREQUENCY_KHZ(x)        (x * MULT_FREQUENCY_KHZ)    // Função constante para frequência de I2C