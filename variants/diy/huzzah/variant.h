// For OLED LCD
#define I2C_SDA 23
#define I2C_SCL 22

// GPS
#undef GPS_RX_PIN
#define GPS_RX_PIN 16

#define BUTTON_PIN 32 // D
#define BUTTON_NEED_PULLUP

#define LORA_DIO0 14 // IRQ E
#define LORA_RESET 33 // B
#define LORA_DIO1 27 // A
#define LORA_DIO2 4 // F
// #define LORA_DIO3

// Not sure if these are needed on RF95W
// #define LORA_RXEN 14 
// #define LORA_TXEN 13

#undef LORA_SCK
#define LORA_SCK 5
#undef LORA_MISO
#define LORA_MISO 19
#undef LORA_MOSI
#define LORA_MOSI 18
#undef LORA_CS
#define LORA_CS 15 // C

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

#define BATTERY_PIN 35
// #define BATTERY_SENSE_SAMPLES 1
#define ADC_CHANNEL ADC1_GPIO35_CHANNEL
#define ADC_MULTIPLIER 2
#define BAT_FULLVOLT 4200
// #define ADC_CTRL 2
// #define ADC_CTRL_ENABLED LOW