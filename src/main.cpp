#include <Arduino.h>
#define FASTLED_INTERNAL // Disable FastLED messages
#include <FastLED.h>
#include <LedStrip.h>
#include <RgbLed.h>
#include <LedUtils.h>
#include <ArduinoUtils.h>

// Xbox error states: https://www.xbox-hq.com/html/xbox-tutorials-163.html
// - Green/Red Flashing
// - Solid Green with no audio/video/eject [still show rainbow]
// - Solid Green with no audio/video [still show rainbow]
// - Orange/Green Flashing
// - Orange Flashing
// - Orange/Red Flashing
// - Solid red
// - Green Flashing

#define DEBUG true
#define DEBUG_VERBOSE false
#define SHOW_ERROR_COLORS true
#define DELAY 15 // delay in ms between rgb changes

// Original LED Signals
#define GREEN_IN_PIN A0
#define RED_IN_PIN A1
bool greenOn = false;
bool redOn = false;
bool isError = false;

// LEDs
#define PORTS_PIN 9
#define LEFT_RING_PIN 8
#define RIGHT_RING_PIN 7
const uint8_t startHue = 160;
LedStrip portsLedStrip(4, PORTS_PIN, 255, startHue);
LedStrip leftRingLedStrip(1, LEFT_RING_PIN, 255, startHue);
LedStrip rightRingLedStrip(1, RIGHT_RING_PIN, 255, startHue);

void showRainbowColor(bool reset) {
    static CRGB rainbowColor;
    rainbowColor = portsLedStrip.rainbowEffect(reset);
    leftRingLedStrip.setColorAll(rainbowColor);
    rightRingLedStrip.setColorAll(rainbowColor);

    #if DEBUG_VERBOSE
    Serial.print(F("rainbowColor ")); Serial.println(LedUtils::CRGBToString(rainbowColor));
    #endif
    
    // Add extra delay when on a solid rgb so it seems smoother (otherwise it will almost immediately change rgb)
    if (rainbowColor.r == 255 || rainbowColor.g == 255 || rainbowColor.b == 255) {
        FastLED.delay(DELAY * 2);
    }
}

void showErrorColor(bool reset) {
    static CRGB prevErrorColor = CRGB::White;
    static CRGB errorColor;
    if (redOn && greenOn) {
        errorColor = CRGB::Orange;
    } else if (!redOn && !greenOn) {
        errorColor = CRGB::Black;
    } else {
        errorColor = redOn ? CRGB::Red : CRGB::Green;
    }
    
    if (reset || prevErrorColor != errorColor) {
        portsLedStrip.setColorAll(errorColor);
        leftRingLedStrip.setColorAll(errorColor);
        rightRingLedStrip.setColorAll(errorColor);

        #if DEBUG_VERBOSE
        Serial.print(F("errorColor ")); Serial.println(LedUtils::CRGBToString(errorColor));
        #endif

        prevErrorColor = errorColor;
    }
}

void checkErrorState() {
    static bool freshBoot = true;
    static bool prevGreenOn = false;
    static bool prevRedOn = false;
    static uint16_t greenValue = 0;
    static uint16_t redValue = 0;
    static uint32_t lastGreenChangeMillis = 0;
    static uint32_t numberOfGreenBlinks = 0;
    static uint32_t lastRedChangeMillis = 0;
    uint16_t currentMillis = millis();

    // Allow 1500ms to detect the error state so that we get the rainbow effect immediately on boot
    if (currentMillis > 1500) {
        freshBoot = false;
    } else {
        return;
    }

    greenValue = analogRead(GREEN_IN_PIN);
    redValue = analogRead(RED_IN_PIN);
    greenOn = greenValue > 450;
    redOn = redValue > 450;

    #if DEBUG_VERBOSE
    Serial.print("redValue: "); Serial.print(redValue);
    Serial.print(" greenValue: "); Serial.print(greenValue);
    Serial.print(" redOn: "); Serial.print(redOn);
    Serial.print(" greenOn: "); Serial.println(greenOn);
    #endif

    if (prevGreenOn != greenOn) {
        #if DEBUG 
        Serial.print(F("Green changed from ")); Serial.print(prevGreenOn); Serial.print(F(" to ")); Serial.print(greenOn);
        Serial.print(F(" time diff: ")); Serial.print(currentMillis - lastGreenChangeMillis); Serial.println(F("ms"));
        #endif

        if (greenOn) {
            numberOfGreenBlinks++;
        }
        lastGreenChangeMillis = currentMillis;
        prevGreenOn = greenOn;
    }

    if (prevRedOn != redOn) {
        #if DEBUG 
        Serial.print(F("Red changed from ")); Serial.print(prevRedOn); Serial.print(F(" to ")); Serial.print(redOn);
        Serial.print(F(" time diff: ")); Serial.print(currentMillis - lastRedChangeMillis); Serial.println(F("ms"));
        #endif

        lastRedChangeMillis = currentMillis;
        prevRedOn = redOn;
    }

    // Detect error states
    if (freshBoot) {
        isError = false;
    } else if (redOn) {
        // If the red LED is on (ring is red or orange), we know it's an error
        isError = true;
    } else if (!redOn && !greenOn) {
        // If neither LED is on (ring is off), it must be blinking, so we know it's an error
        isError = true;
    } else if (!redOn && (currentMillis - lastRedChangeMillis < 1000)) {
        // If red is off, but the last time it was on was less than 1000ms ago, we assume it's still an error because the blink rate is ~200-600ms
        isError = true;
    } else if (numberOfGreenBlinks > 2) {
        // If the green light has blinked at least twice, we know it's an error
        isError = true;
    } else {
        // For all other cases (essentially just solid green), we know it's not an error, or if it is, the Xbox would not have shown error lights
        isError = false;
    }

    #if DEBUG_VERBOSE
    Serial.print(F("redOn: ")); Serial.print(redOn);
    Serial.print(F(" greenOn: ")); Serial.print(greenOn);
    Serial.print(F(" numberOfGreenBlinks: ")); Serial.print(numberOfGreenBlinks);
    Serial.print(F(" isError: ")); Serial.println(isError); 
    #endif

    // Reset the green blink count once it's been solid for 500ms
    if (greenOn && (millis() - lastGreenChangeMillis) > 500) {
        numberOfGreenBlinks = 0;
    }

    #if DEBUG
    static bool prevIsError = false;
    if (prevIsError != isError) {
        Serial.print(F("Error state changed from ")); Serial.print(prevIsError); Serial.print(F(" to ")); Serial.print(isError);
        Serial.print(F(" redOn: ")); Serial.print(redOn); Serial.print(F(" greenOn: ")); Serial.print(greenOn);
        Serial.print(F(" greenValue ")); Serial.print(greenValue); Serial.print(F(" redValue ")); Serial.println(redValue);
        prevIsError = isError;
    }
    #endif
}

void setup() {
    ArduinoUtils::enableFastAnalogSampling();

    #if DEBUG
    Serial.begin(57600);
    #endif
}

void loop() {
    // Check the color of the Xbox LED ring for error modes 
    checkErrorState();

    static bool wasError = false;
    if (isError && SHOW_ERROR_COLORS) {
        // Show normal Xbox error colors if it's in an error state
        showErrorColor(!wasError);
        wasError = true;
    } else {
        // If the light would normally be solid green, show the rainbow effect
        showRainbowColor(wasError);
        wasError = false;
    }

    FastLED.delay(DELAY);
}