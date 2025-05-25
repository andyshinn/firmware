#include "RgbLedIndicatorModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "configuration.h"
#include "main.h"

// Default NeoPixel configuration - can be overridden in variant files
#ifndef NEOPIXEL_INDICATOR_DATA_PIN
#define NEOPIXEL_INDICATOR_DATA_PIN 12
#endif

#ifndef NEOPIXEL_INDICATOR_COUNT
#define NEOPIXEL_INDICATOR_COUNT 1
#endif

#ifndef NEOPIXEL_INDICATOR_TYPE
#ifdef HAS_NEOPIXEL
#define NEOPIXEL_INDICATOR_TYPE (NEO_GRB + NEO_KHZ800)
#else
#define NEOPIXEL_INDICATOR_TYPE NEOPIXEL_TYPE_DEFAULT
#endif
#endif

// Module configuration - can be overridden by uncommenting and modifying
#ifndef NEOPIXEL_INDICATOR_ENABLED
#define NEOPIXEL_INDICATOR_ENABLED true
#endif

#ifndef NEOPIXEL_INDICATOR_BLINK_DURATION_MS
#define NEOPIXEL_INDICATOR_BLINK_DURATION_MS 500
#endif

#ifndef NEOPIXEL_INDICATOR_BRIGHTNESS
#define NEOPIXEL_INDICATOR_BRIGHTNESS 64 // Default to 25% brightness to avoid excessive power draw
#endif

// Which packet types to alert for - can be overridden in variant files
#ifndef NEOPIXEL_INDICATOR_ALERT_TEXT_MESSAGES
#define NEOPIXEL_INDICATOR_ALERT_TEXT_MESSAGES true
#endif

#ifndef NEOPIXEL_INDICATOR_ALERT_POSITION
#define NEOPIXEL_INDICATOR_ALERT_POSITION true
#endif

#ifndef NEOPIXEL_INDICATOR_ALERT_ADMIN
#define NEOPIXEL_INDICATOR_ALERT_ADMIN true
#endif

#ifndef NEOPIXEL_INDICATOR_ALERT_TELEMETRY
#define NEOPIXEL_INDICATOR_ALERT_TELEMETRY true
#endif

#ifndef NEOPIXEL_INDICATOR_ALERT_OTHERS
#define NEOPIXEL_INDICATOR_ALERT_OTHERS true
#endif

RgbLedIndicatorModule *rgbLedIndicatorModule;

RgbLedIndicatorModule::RgbLedIndicatorModule() : MeshModule("RgbLedIndicator"), concurrency::OSThread("RgbLedIndicator")
{
    // Set up as promiscuous to see all packets
    isPromiscuous = true;
}

int32_t RgbLedIndicatorModule::runOnce()
{
    // Check if module is enabled
    if (!NEOPIXEL_INDICATOR_ENABLED) {
        return INT32_MAX; // Disable the thread
    }

#ifndef HAS_NEOPIXEL
    LOG_WARN("NeoPixel Indicator Module enabled but HAS_NEOPIXEL not defined in variant");
    return INT32_MAX; // Disable if NeoPixel support not available
#endif

    if (firstTime) {
        firstTime = false;

        // Set configuration from compile-time defines
        dataPin = NEOPIXEL_INDICATOR_DATA_PIN;
        numLeds = NEOPIXEL_INDICATOR_COUNT;
        pixelType = NEOPIXEL_INDICATOR_TYPE;
        blinkDuration = NEOPIXEL_INDICATOR_BLINK_DURATION_MS;
        brightness = NEOPIXEL_INDICATOR_BRIGHTNESS;

        // Clamp brightness to valid range
        if (brightness > 255)
            brightness = 255;
        if (brightness < 1)
            brightness = 1;

        // Clamp blink duration to valid range (100ms to 5 seconds)
        if (blinkDuration < 100)
            blinkDuration = 100;
        if (blinkDuration > 5000)
            blinkDuration = 5000;

        // Clamp LED count to reasonable range
        if (numLeds > 10)
            numLeds = 10; // Limit to 10 LEDs max for power reasons
        if (numLeds < 1)
            numLeds = 1;

        // Note: We don't initialize the pixels object here since it's managed by AmbientLightingThread
        LOG_INFO("NeoPixel Indicator Module initialized: Pin:%d, Count:%d, Blink:%dms, Brightness:%d", dataPin, numLeds,
                 blinkDuration, brightness);

        // Log alert configuration
        LOG_DEBUG("NeoPixel Alert Configuration:");
        LOG_DEBUG("  Text Messages: %s", NEOPIXEL_INDICATOR_ALERT_TEXT_MESSAGES ? "enabled" : "disabled");
        LOG_DEBUG("  Position: %s", NEOPIXEL_INDICATOR_ALERT_POSITION ? "enabled" : "disabled");
        LOG_DEBUG("  Admin: %s", NEOPIXEL_INDICATOR_ALERT_ADMIN ? "enabled" : "disabled");
        LOG_DEBUG("  Telemetry: %s", NEOPIXEL_INDICATOR_ALERT_TELEMETRY ? "enabled" : "disabled");
        LOG_DEBUG("  Others: %s", NEOPIXEL_INDICATOR_ALERT_OTHERS ? "enabled" : "disabled");
    }

    // Check if we need to turn off the LED
    if (ledActive && millis() > ledOffTime) {
        LOG_DEBUG("NeoPixel: Blink timeout reached - turning off LED (time: %d)", millis());
        turnOffLeds();
        ledActive = false;
    }

    // Run every 50ms to check LED state
    return 50;
}

ProcessMessage RgbLedIndicatorModule::handleReceived(const meshtastic_MeshPacket &mp)
{
#ifdef HAS_NEOPIXEL
    // Using shared global pixels object - no need to check for null
    LOG_DEBUG("NeoPixel: Using shared global pixels object");
#else
    LOG_DEBUG("NeoPixel: Packet ignored - HAS_NEOPIXEL not defined");
    return ProcessMessage::CONTINUE;
#endif

    // Only process packets received from radio (not locally generated)
    if (mp.from == nodeDB->getNodeNum()) {
        LOG_DEBUG("NeoPixel: Packet ignored - locally generated (from: 0x%08x)", mp.from);
        return ProcessMessage::CONTINUE;
    }

    LOG_DEBUG("NeoPixel: Processing packet from 0x%08x, PortNum: %d", mp.from, mp.decoded.portnum);

    // Check if this packet type should trigger an alert
    if (!shouldAlertForPortNum(mp.decoded.portnum)) {
        LOG_DEBUG("NeoPixel: Packet ignored - PortNum %d alerts disabled", mp.decoded.portnum);
        return ProcessMessage::CONTINUE;
    }

    // Get color based on port number
    uint32_t packetColor = getColorForPortNum(mp.decoded.portnum);

    LOG_DEBUG("NeoPixel: Triggering blink for PortNum %d (Color: 0x%06X, R:%d G:%d B:%d)", mp.decoded.portnum, packetColor,
              (packetColor >> 16) & 0xFF, // Red
              (packetColor >> 8) & 0xFF,  // Green
              packetColor & 0xFF);        // Blue

    // Trigger LED blink
    triggerBlink(packetColor);

    return ProcessMessage::CONTINUE;
}

bool RgbLedIndicatorModule::shouldAlertForPortNum(meshtastic_PortNum portnum)
{
    switch (portnum) {
    case meshtastic_PortNum_TEXT_MESSAGE_APP:
    case meshtastic_PortNum_TEXT_MESSAGE_COMPRESSED_APP:
    case meshtastic_PortNum_ALERT_APP:
        return NEOPIXEL_INDICATOR_ALERT_TEXT_MESSAGES;

    case meshtastic_PortNum_POSITION_APP:
    case meshtastic_PortNum_NODEINFO_APP:
        return NEOPIXEL_INDICATOR_ALERT_POSITION;

    case meshtastic_PortNum_ADMIN_APP:
    case meshtastic_PortNum_ROUTING_APP:
        return NEOPIXEL_INDICATOR_ALERT_ADMIN;

    case meshtastic_PortNum_TELEMETRY_APP:
        return NEOPIXEL_INDICATOR_ALERT_TELEMETRY;

    default:
        return NEOPIXEL_INDICATOR_ALERT_OTHERS;
    }
}

void RgbLedIndicatorModule::setPixelColor(uint32_t color)
{
#ifdef HAS_NEOPIXEL
    LOG_DEBUG("NeoPixel: Setting %d LEDs to color 0x%06X", numLeds, color);
    // Set all LEDs to the same color using global pixels object
    for (int i = 0; i < numLeds; i++) {
        pixels.setPixelColor(i, color);
    }
    pixels.show();
#endif
}

void RgbLedIndicatorModule::turnOffLeds()
{
#ifdef HAS_NEOPIXEL
    LOG_DEBUG("NeoPixel: Restoring ambient lighting");
    restoreAmbientState();
#endif
}

uint32_t RgbLedIndicatorModule::getColorForPortNum(meshtastic_PortNum portnum)
{
    switch (portnum) {
    // Red: Text messages and alerts
    case meshtastic_PortNum_TEXT_MESSAGE_APP:
    case meshtastic_PortNum_TEXT_MESSAGE_COMPRESSED_APP:
    case meshtastic_PortNum_ALERT_APP:
        return color(255, 0, 0);

    // Green: Position
    case meshtastic_PortNum_POSITION_APP:
        return color(0, 255, 0);

    // Blue: Node info
    case meshtastic_PortNum_NODEINFO_APP:
        return color(0, 0, 255);

    // Yellow: Telemetry
    case meshtastic_PortNum_TELEMETRY_APP:
        return color(255, 255, 0);

    // Purple: Admin and routing
    case meshtastic_PortNum_ROUTING_APP:
        return color(255, 0, 255);

    // Cyan: Serial and range test
    case meshtastic_PortNum_SERIAL_APP:
    case meshtastic_PortNum_RANGE_TEST_APP:
        return color(0, 255, 255);

    // Orange: Hardware and admin
    case meshtastic_PortNum_ADMIN_APP:
    case meshtastic_PortNum_REMOTE_HARDWARE_APP:
        return color(255, 165, 0);

    // Pink: Traceroutes
    case meshtastic_PortNum_TRACEROUTE_APP:
        return color(255, 192, 203);

    // Light Blue: Neighbor info
    case meshtastic_PortNum_NEIGHBORINFO_APP:
        return color(173, 216, 230);

    // Default: White for unknown/other port numbers
    default:
        return color(255, 255, 255);
    }
}

void RgbLedIndicatorModule::triggerBlink(uint32_t color)
{
    LOG_DEBUG("NeoPixel: Starting blink - Color: 0x%06X, Duration: %dms", color, blinkDuration);

    // Save current ambient lighting state before overriding
    saveAmbientState();

    // Store the color
    currentColor = color;

    // Turn on LED
    setPixelColor(currentColor);

    // Set when to turn it off
    ledOffTime = millis() + blinkDuration;
    ledActive = true;

    LOG_DEBUG("NeoPixel: LED will turn off at %d (current: %d)", ledOffTime, millis());
}

uint32_t RgbLedIndicatorModule::applyBrightness(uint32_t color, uint8_t brightness)
{
    // Extract RGB components
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    // Apply brightness scaling
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    // Recombine into 32-bit color
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

uint32_t RgbLedIndicatorModule::color(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void RgbLedIndicatorModule::saveAmbientState()
{
#ifdef HAS_NEOPIXEL
    // Get the current color from the first pixel (assuming all pixels have same ambient color)
    savedAmbientColor = pixels.getPixelColor(0);
    LOG_DEBUG("NeoPixel: Saved ambient color: 0x%06X", savedAmbientColor);
#endif
}

void RgbLedIndicatorModule::restoreAmbientState()
{
#ifdef HAS_NEOPIXEL
    // Restore the saved ambient color to all pixels
    for (int i = 0; i < numLeds; i++) {
        pixels.setPixelColor(i, savedAmbientColor);
    }
    pixels.show();
    LOG_DEBUG("NeoPixel: Restored ambient color: 0x%06X", savedAmbientColor);
#endif
}