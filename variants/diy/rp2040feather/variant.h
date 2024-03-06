// #define ARDUINO_ARCH_AVR

// For OLED LCD
#define I2C_SDA SDA
#define I2C_SCL SCL

// GPS
#define GPS_RX_PIN PIN_SERIAL1_RX

// #define BUTTON_PIN 32 // D
// #define BUTTON_NEED_PULLUP

#define LORA_DIO0 D5   // IRQ E
#define LORA_RESET D10 // B
#define LORA_DIO1 D11  // A
#define LORA_DIO2 D25  // F
// #define LORA_DIO3

// Not sure if these are needed on RF95W
// #define LORA_RXEN 14
// #define LORA_TXEN 13

#define LORA_SCK SCK
#define LORA_MISO MISO
#define LORA_MOSI MOSI
#define LORA_CS D9

// RX/TX for RFM95/SX127x
// #define RF95_RXEN LORA_RXEN
// #define RF95_TXEN LORA_TXEN
// #define RF95_TCXO <GPIO#>

// common pinouts for SX126X modules
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET
// #define SX126X_RXEN LORA_RXEN
// #define SX126X_TXEN LORA_TXEN

// supported modules list
#define USE_RF95 // RFM95/SX127x
#define USE_SX1262
// #define USE_SX1268
// #define USE_LLCC68
// #define USE_SH1107_128_64

#define BATTERY_PIN A0
#define BATTERY_SENSE_RESOLUTION_BITS ADC_RESOLUTION
#define ADC_CHANNEL ADC1_GPIO35_CHANNEL
#define ADC_MULTIPLIER 2
#define BAT_FULLVOLT 4200
// #define ADC_CTRL 2
// #define ADC_CTRL_ENABLED LOW