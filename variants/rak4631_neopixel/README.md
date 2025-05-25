# RAK 4631 NeoPixel Variant

This variant adds NeoPixel/WS2812 RGB LED support to the RAK 4631 board, including the RGB LED Packet Indicator Module.

## Features

- Full NeoPixel/WS2812 addressable LED support
- Visual packet indication with color-coded alerts
- Configurable brightness and blink patterns
- Support for single or multiple chained LEDs
- Low power operation (default 25% brightness)

## Hardware Configuration

### Default Pin Assignment

- **Data Pin**: SDA (GPIO 13) - I2C SDA pin, safe to use if no I2C devices are connected
- **LED Count**: 1 (configurable)
- **LED Type**: WS2812/WS2812B (GRB, 800kHz)

### Wiring

Connect your NeoPixel to the WisBlock base board:

```
RAK5005-O SDA Pin ──────── NeoPixel Data In
3.3V/5V ─────────────── NeoPixel VCC
GND ─────────────────── NeoPixel GND
```

**Note**: For 5V NeoPixels with the 3.3V RAK 4631, consider using a level shifter for reliable data transmission.

### Alternative Pins

You can modify the pin assignment by changing these defines in `variant.h`:

```cpp
#define NEOPIXEL_INDICATOR_DATA_PIN 13  // Change to desired GPIO pin
```

Available pins (if not using I2C):

- SDA → GPIO 13 (default)
- SCL → GPIO 14

Available WisBlock GPIO pins (if accessible):

- IO1 → GPIO 17
- IO2 → GPIO 34
- IO3 → GPIO 21
- IO4 → GPIO 4
- IO5 → GPIO 9
- IO6 → GPIO 10
- IO7 → GPIO 28

## Packet Color Coding

The RGB LED Indicator Module provides visual feedback for different packet types:

| Packet Type                 | Color      |
| --------------------------- | ---------- |
| Text Messages & Alerts      | Red        |
| Position & Node Info        | Green      |
| Admin & Routing             | Blue       |
| Telemetry                   | Yellow     |
| Audio & Detection           | Purple     |
| Serial & Range Test         | Cyan       |
| Remote Hardware             | Orange     |
| Waypoints                   | Pink       |
| Neighbor Info & Trace Route | Light Blue |
| Other/Unknown               | White      |

## Configuration Options

You can customize the behavior by modifying these defines in `variant.h`:

```cpp
// Hardware configuration
#define NEOPIXEL_INDICATOR_COUNT 1           // Number of LEDs
#define NEOPIXEL_INDICATOR_BRIGHTNESS 64     // Brightness (1-255)
#define NEOPIXEL_INDICATOR_BLINK_DURATION_MS 500  // Blink duration

// Alert types (true/false)
#define NEOPIXEL_INDICATOR_ALERT_TEXT_MESSAGES true
#define NEOPIXEL_INDICATOR_ALERT_POSITION true
#define NEOPIXEL_INDICATOR_ALERT_ADMIN true
#define NEOPIXEL_INDICATOR_ALERT_TELEMETRY true
#define NEOPIXEL_INDICATOR_ALERT_OTHERS true
```

## Building

To build this variant:

```bash
# Standard build
pio run -e rak4631_neopixel

# Debug build
pio run -e rak4631_neopixel_dbg
```

## Power Considerations

- **Single LED**: ~5mA @ 25% brightness, ~20mA @ full brightness
- **Multiple LEDs**: Scale accordingly
- **External power recommended** for >5 LEDs or brightness >50%

## Compatibility

This variant is compatible with:

- Standard WS2812/WS2812B NeoPixels
- NeoPixel strips, rings, and matrices
- 3.3V and 5V NeoPixels (with appropriate level shifting)
- All standard RAK WisBlock modules and accessories

## Based On

This variant extends the standard `rak4631` variant with additional NeoPixel support while maintaining full compatibility with all existing RAK 4631 features.

## Debug Logging

The NeoPixel module includes comprehensive debug logging to help troubleshoot issues:

### Enabling Debug Logs

To see debug output, you'll need to:

1. **Use the debug build variant**:

   ```bash
   pio run -e rak4631_neopixel_dbg
   ```

2. **Connect serial monitor** to view logs:
   ```bash
   pio device monitor -e rak4631_neopixel_dbg
   ```

### Debug Log Messages

You'll see messages like:

```
NeoPixel Indicator Module initialized: Pin:13, Count:1, Blink:500ms, Brightness:64
NeoPixel Alert Configuration:
  Text Messages: enabled
  Position: enabled
  Admin: enabled
  Telemetry: enabled
  Others: enabled
NeoPixel: Processing packet from 0x12345678, PortNum: 1
NeoPixel: Triggering blink for PortNum 1 (Color: 0xFF0000, R:255 G:0 B:0)
NeoPixel: Starting blink - Color: 0xFF0000, Duration: 500ms
NeoPixel: Setting 1 LEDs to color 0xFF0000
NeoPixel: LED will turn off at 12345 (current: 11845)
NeoPixel: Blink timeout reached - turning off LED (time: 12346)
NeoPixel: Turning off all LEDs
```

### Common Debug Messages

- **Packet ignored** messages help identify why certain packets don't trigger LEDs
- **Color information** shows RGB values for troubleshooting color issues
- **Timing information** helps debug blink duration and LED state
- **Initialization logs** confirm configuration and hardware setup
