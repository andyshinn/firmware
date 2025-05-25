#pragma once
#include "MeshModule.h"
#include "concurrency/OSThread.h"
#include "configuration.h"

#ifdef HAS_NEOPIXEL
#include <Adafruit_NeoPixel.h>
// Define NeoPixel constants if we have NeoPixel support
#ifndef NEOPIXEL_TYPE_DEFAULT
#define NEOPIXEL_TYPE_DEFAULT (NEO_GRB + NEO_KHZ800)
#endif
#else
// Define fallback constants for when NeoPixel is not available
#define NEO_GRB 0x01
#define NEO_KHZ800 0x0000
#define NEOPIXEL_TYPE_DEFAULT 0x0001
#endif

/**
 * RGB LED Packet Indicator Module
 *
 * This module blinks an RGB LED whenever a packet is received over the radio.
 * The color of the LED corresponds to the PortNum of the received packet:
 * - Red: Text messages and alerts (TEXT_MESSAGE_APP, ALERT_APP)
 * - Green: Position and node info (POSITION_APP, NODEINFO_APP)
 * - Blue: Admin and routing (ADMIN_APP, ROUTING_APP)
 * - Yellow: Telemetry (TELEMETRY_APP)
 * - Purple: Audio and detection sensor (AUDIO_APP, DETECTION_SENSOR_APP)
 * - Cyan: Serial and range test (SERIAL_APP, RANGE_TEST_APP)
 * - White: Unknown or other port numbers
 *
 * Uses NeoPixel/WS2812 addressable LEDs for better color control and easier wiring.
 * Only requires a single GPIO pin instead of three separate RGB pins.
 */
class RgbLedIndicatorModule : public MeshModule, public concurrency::OSThread
{
  public:
    /** Constructor */
    RgbLedIndicatorModule();

  protected:
    /** Called to handle a particular incoming message */
    virtual ProcessMessage handleReceived(const meshtastic_MeshPacket &mp) override;

    /** We want to see all packets regardless of port number */
    virtual bool wantPacket(const meshtastic_MeshPacket *p) override { return true; }

    /** Thread function for LED blinking */
    virtual int32_t runOnce() override;

  private:
    // NeoPixel configuration
    uint8_t dataPin = 0;
    uint8_t numLeds = 1;
    uint16_t pixelType = NEOPIXEL_TYPE_DEFAULT;

    // Configuration values
    uint32_t blinkDuration = 500; // ms
    uint8_t brightness = 255;     // 1-255
    bool firstTime = true;

    // LED state tracking
    bool ledActive = false;
    uint32_t ledOffTime = 0;
    uint32_t currentColor = 0;

#ifdef HAS_NEOPIXEL
    Adafruit_NeoPixel *pixels = nullptr;
#endif

    /** Set NeoPixel color (32-bit RGB color) */
    void setPixelColor(uint32_t color);

    /** Turn off NeoPixel */
    void turnOffLeds();

    /** Get 32-bit RGB color for a specific port number */
    uint32_t getColorForPortNum(meshtastic_PortNum portnum);

    /** Trigger LED blink with specified color */
    void triggerBlink(uint32_t color);

    /** Check if an alert should be triggered for a specific port number */
    bool shouldAlertForPortNum(meshtastic_PortNum portnum);

    /** Apply brightness scaling to a color */
    uint32_t applyBrightness(uint32_t color, uint8_t brightness);

    /** Create 32-bit color from RGB values */
    uint32_t color(uint8_t r, uint8_t g, uint8_t b);
};

extern RgbLedIndicatorModule *rgbLedIndicatorModule;