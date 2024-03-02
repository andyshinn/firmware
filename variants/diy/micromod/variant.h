// For OLED LCD
#define I2C_SDA SDA
#define I2C_SCL SCL

#define LORA_DIO0 D0 // F0
#define LORA_RESET G1 // F4
#define LORA_DIO1 G2 // F5
#define LORA_DIO2 G3 // F6

#define LORA_RXEN G0 // F3
#define LORA_TXEN PWM0 // F2
#define LORA_SCK SCK
#define LORA_MISO MISO
#define LORA_MOSI MOSI
#define LORA_CS SS // F1 / SPI_CS0

// RX/TX for RFM95/SX127x
#define RF95_RXEN LORA_RXEN
#define RF95_TXEN LORA_TXEN

// common pinouts for SX126X modules
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_DIO2
#define SX126X_RESET LORA_RESET
#define SX126X_RXEN LORA_RXEN
#define SX126X_TXEN LORA_TXEN

// supported modules list
#define USE_RF95
#define USE_SX1262
#define HAS_GPS 0
#define HAS_SCREEN 0

// There is no battery monitor pin on the MicroMod main boards
#undef HAS_TELEMETRY
