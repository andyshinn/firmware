# RAK 4631 NeoPixel Variant - Build Test Guide

## Quick Test Commands

### 1. Check Environment Configuration

```bash
# Verify the environment is detected
pio project config | grep -A 10 "env:rak4631_neopixel"
```

### 2. Clean Build Test

```bash
# Clean any previous builds
pio run -e rak4631_neopixel -t clean

# Test build (this will download dependencies and compile)
pio run -e rak4631_neopixel
```

### 3. Debug Build Test

```bash
# Test debug variant
pio run -e rak4631_neopixel_dbg
```

## Expected Build Output

If successful, you should see:

- Adafruit NeoPixel library downloaded and compiled
- No compilation errors related to NeoPixel definitions
- Firmware binary created in `.pio/build/rak4631_neopixel/`

## Hardware Test Setup

### Wiring

Connect a WS2812/NeoPixel to RAK5005-O base board:

```
SDA Pin (I2C) ──────── NeoPixel Data In
3.3V or 5V ─────────── NeoPixel VCC
GND ──────────────── NeoPixel GND
```

### Expected Behavior

After flashing firmware:

1. LED should blink briefly on startup (initialization)
2. LED should flash different colors when packets are received:
   - **Red**: Text messages
   - **Green**: Position updates
   - **Blue**: Admin commands
   - **Yellow**: Telemetry
   - **Other colors**: Various packet types

### Verification

1. Send a text message → LED should flash red
2. Request position → LED should flash green
3. Send admin command → LED should flash blue

## Configuration Customization

To modify LED behavior, edit `variants/rak4631_neopixel/variant.h`:

```cpp
// Change pin (if using different IO pin)
#define NEOPIXEL_INDICATOR_DATA_PIN 21  // Use IO3 instead of IO1

// Change number of LEDs
#define NEOPIXEL_INDICATOR_COUNT 8      // For LED strip with 8 pixels

// Change brightness (1-255)
#define NEOPIXEL_INDICATOR_BRIGHTNESS 128  // 50% brightness

// Change blink duration
#define NEOPIXEL_INDICATOR_BLINK_DURATION_MS 1000  // 1 second blink
```

## Troubleshooting

### Build Errors

- **"HAS_NEOPIXEL not defined"**: Check that variant.h includes the NeoPixel defines
- **"Adafruit_NeoPixel.h not found"**: Verify library is in platformio.ini lib_deps
- **Pin conflicts**: Ensure chosen GPIO pin isn't used by other modules

### Runtime Issues

- **No LED activity**: Check wiring and power supply
- **Dim/flickering LED**: Add 1000µF capacitor across power supply
- **Inconsistent colors**: Use level shifter for 5V LEDs with 3.3V board

### Debug Mode

Use the debug variant for troubleshooting:

```bash
pio run -e rak4631_neopixel_dbg
```

This enables:

- Serial debug output
- LED initialization messages
- Packet reception logging

## Monitoring Debug Logs

### Real-time Monitoring

```bash
# Build and upload debug firmware, then monitor logs
pio run -e rak4631_neopixel_dbg -t upload && pio device monitor -e rak4631_neopixel_dbg
```

### Key Log Messages to Watch For

**Initialization Success:**

```
NeoPixel Indicator Module initialized: Pin:13, Count:1, Blink:500ms, Brightness:64
```

**Packet Processing:**

```
NeoPixel: Processing packet from 0x12345678, PortNum: 1
NeoPixel: Triggering blink for PortNum 1 (Color: 0xFF0000, R:255 G:0 B:0)
```

**LED Activity:**

```
NeoPixel: Starting blink - Color: 0xFF0000, Duration: 500ms
NeoPixel: Setting 1 LEDs to color 0xFF0000
NeoPixel: Blink timeout reached - turning off LED
```

**Common Issues:**

```
NeoPixel: Packet ignored - locally generated
NeoPixel: Packet ignored - PortNum 67 alerts disabled
NeoPixel: Cannot set color - pixels object is null
```

### Quick Debug Commands

```bash
# Just monitor logs without rebuilding
pio device monitor -e rak4631_neopixel_dbg

# Build, upload and monitor in one command
pio run -e rak4631_neopixel_dbg -t upload -t monitor
```
