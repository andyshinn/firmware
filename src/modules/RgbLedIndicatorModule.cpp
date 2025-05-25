#include "RgbLedIndicatorModule.h"
#include "MeshService.h"
#include "NodeDB.h"
#include "Router.h"
#include "configuration.h"
#include "main.h"
#include "mesh/generated/meshtastic/module_config.pb.h"

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

// Duplicate packet detection configuration
#ifndef NEOPIXEL_INDICATOR_ALERT_DUPLICATES
#define NEOPIXEL_INDICATOR_ALERT_DUPLICATES true
#endif

RgbLedIndicatorModule *rgbLedIndicatorModule;

RgbLedIndicatorModule::RgbLedIndicatorModule() : MeshModule("RgbLedIndicator"), concurrency::OSThread("RgbLedIndicator")
{
    // Set up as promiscuous to see all packets
    isPromiscuous = true;

    // Initialize saved ambient colors to black (off)
    for (int i = 0; i < 10; i++) {
        savedAmbientColor[i] = 0;
    }

    // Initialize duplicate detection statistics
    lastRxDupe = 0;
    lastTxRelayCanceled = 0;
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
        LOG_DEBUG("  Duplicates: %s", NEOPIXEL_INDICATOR_ALERT_DUPLICATES ? "enabled" : "disabled");
    }

    // Check for duplicate packets via router statistics
    checkForDuplicateStats();

    // Handle LED state management with new timing system
    if (blinkStartTime > 0) {
        uint32_t elapsed = millis() - blinkStartTime;
        if (elapsed >= blinkDuration) {
            // Blink duration has elapsed, restore ambient state
            restoreAmbientState();
            blinkStartTime = 0;
            LOG_DEBUG("NeoPixel: Blink completed, ambient state restored");
        }
    }

    // Run every 100ms for responsive duplicate detection
    return 100;
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

    LOG_DEBUG("NeoPixel: Processing packet from 0x%08x, PortNum: %d, ID: 0x%08x", mp.from, mp.decoded.portnum, mp.id);

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
    // Get the actual number of LEDs from the pixels object
    int actualLedCount = pixels.numPixels();
    int ledsToSet = (numLeds < actualLedCount) ? numLeds : actualLedCount;

    // Temporarily increase brightness for packet alerts to make them visible
    pixels.setBrightness(brightness);

    // Extract RGB components from our color value
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    // Use pixels.Color() method for proper color handling
    uint32_t pixelColor = pixels.Color(r, g, b);

    LOG_DEBUG("NeoPixel: Setting %d LEDs to color 0x%06X (RGB: %d,%d,%d) brightness: %d", ledsToSet, color, r, g, b, brightness);

    // Set LEDs to the packet alert color
    for (int i = 0; i < ledsToSet; i++) {
        pixels.setPixelColor(i, pixelColor);
    }
    pixels.show();
    delay(1); // Small delay to ensure LED controller processes the change
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
#ifdef HAS_NEOPIXEL
    // Save current ambient state before changing colors
    saveAmbientState();

    // Set the new color
    setPixelColor(color);

    // Record when the blink started
    blinkStartTime = millis();

    LOG_DEBUG("NeoPixel: Blink triggered with color 0x%06X, duration %dms", color, blinkDuration);
#endif
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
    // Instead of saving pixel colors, we'll let AmbientLightingThread handle restoration
    // Just save the current brightness setting
    savedAmbientColor[0] = pixels.getBrightness(); // Store brightness in first element
    LOG_DEBUG("NeoPixel: Saved ambient brightness: %d", (int)savedAmbientColor[0]);
#endif
}

void RgbLedIndicatorModule::restoreAmbientState()
{
#ifdef HAS_NEOPIXEL
    // First restore the brightness
    pixels.setBrightness((uint8_t)savedAmbientColor[0]);

    // Check if ambient lighting is enabled
    if (moduleConfig.ambient_lighting.led_state) {
        // Ambient lighting is enabled - restore ambient colors using moduleConfig (same as AmbientLightingThread)
        pixels.fill(pixels.Color(moduleConfig.ambient_lighting.red, moduleConfig.ambient_lighting.green,
                                 moduleConfig.ambient_lighting.blue),
                    0, NEOPIXEL_COUNT);

        LOG_DEBUG("NeoPixel: Restored ambient lighting from moduleConfig R:%d G:%d B:%d Brightness:%d",
                  moduleConfig.ambient_lighting.red, moduleConfig.ambient_lighting.green, moduleConfig.ambient_lighting.blue,
                  (int)savedAmbientColor[0]);
    } else {
        // Ambient lighting is disabled - turn off all LEDs
        pixels.clear();
        LOG_DEBUG("NeoPixel: Ambient lighting disabled - cleared all LEDs");
    }

    pixels.show();
    delay(1); // Small delay to ensure LED controller processes the change
#endif
}

void RgbLedIndicatorModule::checkForDuplicateStats()
{
    if (!router || !NEOPIXEL_INDICATOR_ALERT_DUPLICATES) {
        return;
    }

    // Check if router duplicate statistics have increased
    uint32_t currentRxDupe = router->rxDupe;
    uint32_t currentTxRelayCanceled = router->txRelayCanceled;

    if (currentRxDupe > lastRxDupe) {
        // New duplicate packet(s) detected
        uint32_t newDupes = currentRxDupe - lastRxDupe;
        LOG_DEBUG("NeoPixel: %d new duplicate packet(s) detected (total: %d)", newDupes, currentRxDupe);

        // Show duplicate with special color (orange/amber)
        uint32_t duplicateColor = color(255, 140, 0); // Orange/amber for duplicates
        LOG_DEBUG("NeoPixel: Triggering duplicate blink (Color: 0x%06X)", duplicateColor);
        triggerBlink(duplicateColor);

        lastRxDupe = currentRxDupe;
    }

    if (currentTxRelayCanceled > lastTxRelayCanceled) {
        // New relay cancellation(s) due to duplicates
        uint32_t newCanceled = currentTxRelayCanceled - lastTxRelayCanceled;
        LOG_DEBUG("NeoPixel: %d new relay cancellation(s) due to duplicates (total: %d)", newCanceled, currentTxRelayCanceled);

        lastTxRelayCanceled = currentTxRelayCanceled;
    }
}
