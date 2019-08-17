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

#define DEBUG false
#define SHOW_ERROR_COLORS true
#define DELAY 15 // delay in ms between rgb changes

// Original LED Signals
#define GREEN_IN_PIN A0
#define RED_IN_PIN A1
bool greenOn = false;
bool redOn = false;
bool isError = false;

// LEDs
const uint8_t startHue = 160;
LedStrip ledStrip(4, 13, 255, startHue);
RgbLed ringLedLeft(3, 5, 6);
RgbLed ringLedRight(9, 10, 11);

void showRainbowColor() {
    static CRGB rainbowColor;
    rainbowColor = ledStrip.rainbowEffect();
    ringLedLeft.setColor(rainbowColor);
    ringLedRight.setColor(rainbowColor);

    // #if DEBUG
    // Serial.print(F("rainbowColor ")); Serial.println(LedUtils::CRGBToString(rainbowColor));
    // #endif
    
    // Add extra delay when on a solid rgb so it seems smoother (otherwise it will almost immediately change rgb)
    if (rainbowColor.r == 255 || rainbowColor.g == 255 || rainbowColor.b == 255) {
        FastLED.delay(DELAY * 2);
    }
}

void showErrorColor() {
    static CRGB prevErrorColor;
    static CRGB errorColor;
    errorColor = CRGB(redOn ? 255 : 0, greenOn ? 255: 0, 0);
    if (prevErrorColor != errorColor) {
        ledStrip.colorAllLeds(errorColor);
        ringLedLeft.setColor(errorColor);
        ringLedRight.setColor(errorColor);

        #if DEBUG
        Serial.print(F("errorColor ")); Serial.println(LedUtils::CRGBToString(errorColor));
        #endif

        prevErrorColor = errorColor;
    }
}

void checkErrorState() {
    static bool prevGreenOn = false;
    static bool prevRedOn = false;
    static uint16_t greenValue = 0;
    static uint16_t redValue = 0;
    static uint32_t lastGreenChangeMillis = 0;
    static uint32_t numberOfGreenBlinks = 0;
    static uint32_t lastRedChangeMillis = 0;
    static uint32_t lastRedOnMillis = 0;
    static uint32_t numberOfRedBlinks = 0;

    greenValue = analogRead(GREEN_IN_PIN);
    redValue = analogRead(RED_IN_PIN);
    greenOn = greenValue > 450;
    redOn = redValue > 450;

    // #if DEBUG
    // Serial.print("redValue: "); Serial.print(redValue);
    // Serial.print(" greenValue: "); Serial.print(greenValue);
    // Serial.print(" redOn: "); Serial.print(redOn);
    // Serial.print(" greenOn: "); Serial.println(greenOn);
    // #endif

    if (prevGreenOn != greenOn) {
        #if DEBUG 
        Serial.print(F("Green changed from ")); Serial.print(prevGreenOn); Serial.print(F(" to ")); Serial.print(greenOn);
        Serial.print(F(" time diff: ")); Serial.print(millis() - lastGreenChangeMillis); Serial.println(F("ms"));
        #endif

        if (greenOn) {
            numberOfGreenBlinks++;
        }
        lastGreenChangeMillis = millis();
        prevGreenOn = greenOn;
    }

    if (prevRedOn != redOn) {
        #if DEBUG 
        Serial.print(F("Red changed from ")); Serial.print(prevRedOn); Serial.print(F(" to ")); Serial.print(redOn);
        Serial.print(F(" time diff: ")); Serial.print(millis() - lastRedChangeMillis); Serial.println(F("ms"));
        #endif

        if (redOn) {
            numberOfRedBlinks++;
            lastRedOnMillis = millis();
        }
        lastRedChangeMillis = millis();
        prevRedOn = redOn;
    }

    // Detect error states
    uint16_t currentMillis = millis();
    if (currentMillis < 1500) {
        // Allow a 1500ms to detect the error state
        isError = false;
    } else if (redOn) {
        // If the red LED is on (ring is red or orange), we know it's an error
        isError = true;
    } else if (!redOn && !greenOn) {
        // If neither LED is on (ring is off), it must be blinking, so we know it's an error
        isError = true;
    } else if (millis() - lastRedOnMillis < 1000) {
        // If red is off, but the last time it was on was less than 1000ms ago, we know it's an error because the blink rate is ~200-600ms
        isError = true;
    } else if (numberOfGreenBlinks > 2) {
        // If the green light has blinked at least twice, we know it's an error
        isError = true;
    } else {
        // For all other cases (essentially just solid green), we know it's not an error, or if it is, the Xbox would not have shown error lights
        isError = false;

        // #if DEBUG
        // Serial.print("redOn: "); Serial.print(redOn);
        // Serial.print(" greenOn: "); Serial.print(greenOn);
        // Serial.print(" millis() - lastRedOnMillis: "); Serial.print(millis() - lastRedOnMillis);
        // Serial.print(" numberOfGreenBlinks: "); Serial.print(numberOfGreenBlinks);
        // Serial.print(" isError: "); 
        // #endif
    }

    // Reset the green blink count once it's been solid for 500ms
    if (greenOn && (millis() - lastGreenChangeMillis) > 500) {
        numberOfGreenBlinks = 0;
    }

    #if DEBUG
    static bool prevIsError = false;
    if (prevIsError != isError) {
        Serial.print(F("Error state changed from "));
        Serial.print(prevIsError);
        Serial.print(F(" to "));
        Serial.println(isError);
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

    if (isError && SHOW_ERROR_COLORS) {
        // Show normal Xbox error colors if it's in an error state 
        showErrorColor();
    } else {
        // If the light would normally be solid green, show the rainbow effect
        showRainbowColor();
    }

    FastLED.delay(DELAY);
}