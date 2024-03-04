// For OLED LCD
// #define I2C_SDA 21 // PIN_WIRE_SDA
// #define I2C_SCL 22 // PIN_WIRE_SDL
#define WIRE_INTERFACES_COUNT 1
#define SPI_INTERFACES_COUNT 1
#define HAS_SCREEN 0

// #define PIN_SPI_MOSI 24
// #define PIN_SPI_MISO 23
// #define PIN_SPI_SCK 25
// #define PIN_WIRE_SDA 16
// #define PIN_WIRE_SCL 14
// #define LED_BUILTIN 6

// GPS
#define GPS_RX_PIN 0 // PIN_SERIAL1_RX

#define BUTTON_PIN 4
// #define BUTTON_NEED_PULLUP
// #define PIN_LED1 3

#define LORA_DIO0 5 // IRQ E
#define LORA_RESET 7 // B
#define LORA_DIO1 9 // A
#define LORA_DIO2 10 // F
// #define LORA_DIO3

// Not sure if these are needed on RF95W
// #define LORA_RXEN 14 
// #define LORA_TXEN 13

#define LORA_SCK 13
#define LORA_MISO 20
#define LORA_MOSI 15
#define LORA_CS 5

// RX/TX for RFM95/SX127x
// #define RF95_RXEN LORA_RXEN
// #define RF95_TXEN LORA_TXEN
// #define RF95_TCXO <GPIO#>

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET
// #define SX126X_RXEN LORA_RXEN
// #define SX126X_TXEN LORA_TXEN

#define USE_RF95
#define USE_SX1262

#define BATTERY_PIN 14 // A0
#define BATTERY_SENSE_RESOLUTION_BITS 12
#define ADC_MULTIPLIER 2

// #define PIN_SERIAL2_RX 8
// #define PIN_SERIAL2_TX 6
