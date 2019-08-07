#include <Arduino.h>
#include <FastLED.h>

#define DEBUG false

const uint8_t startHue = 160;
const unsigned long delayMillis = 15; // delay in ms between rgb changes

// Common Cathode RGB LEDs
//
// LED 1
const uint8_t redPin1 = 3;
const uint8_t greenPin1 = 5;
const uint8_t bluePin1 = 6;
// LED 2
const uint8_t redPin2 = 9;
const uint8_t greenPin2 = 10;
const uint8_t bluePin2 = 11;

// Addressable LEDs
#define LED_TYPE NEOPIXEL
#define NUM_LEDS 4
#define DEFAULT_BRIGHTNESS 255
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

void setColor(CRGB rgb) {
    // Common Cathode RGB LEDs
    analogWrite(redPin1, rgb.r); analogWrite(greenPin1, rgb.g); analogWrite(bluePin1, rgb.b);
    analogWrite(redPin2, rgb.r); analogWrite(greenPin2, rgb.g); analogWrite(bluePin2, rgb.b);

    // Addressable LEDs
    fill_solid(leds, NUM_LEDS, rgb);

    #if DEBUG
    Serial.print("r: ");
    Serial.print(rgb.r);
    Serial.print(" g: ");
    Serial.print(rgb.g);
    Serial.print(" b: ");
    Serial.println(rgb.b);
    #endif

    // Add extra delay when on a solid rgb so it seems smoother (otherwise it will almost immediately change rgb)
    if (rgb.r == 255 || rgb.g == 255 || rgb.b == 255) {
        FastLED.delay(delayMillis * 2);
    }
}

void setup() {
    #if DEBUG
    Serial.begin(57600);
    #endif

    FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

// Better color smoothing code from https://www.reddit.com/r/arduino/comments/5o552l/fastled_how_can_i_create_a_smoothnonchoppy_color/dcgnuje/
void loop() {
    static CRGB currentColor = CRGB(CHSV(startHue, 255, 255));
    static CRGB lastColor = currentColor;
    static CRGB targetColor = currentColor;
    static uint8_t hue = startHue;
    static uint8_t step = 0;

    currentColor = blend(lastColor, targetColor, 255 / 8 * step++);
    if(step > 8) {
        step = 0;
        lastColor = targetColor;
        hsv2rgb_rainbow(CHSV(hue++, 255, 255), targetColor);
    }
    setColor(currentColor);

    FastLED.delay(delayMillis);
}