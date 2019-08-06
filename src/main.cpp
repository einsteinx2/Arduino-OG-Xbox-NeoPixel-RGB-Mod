#include <Arduino.h>

#define SPEED 35 // delay in ms between color changes

//Credit to https://gist.github.com/jamesotron/ for original code
//I simply modified to run 2 LED's and run the colour changes a bit slower
// const int redPin1 = 6;
// const int greenPin1 = 5;
// const int bluePin1 = 3;

const int redPin2 = 10;
const int greenPin2 = 11;
const int bluePin2 = 9;

void setColorRgb(unsigned int red, unsigned int green, unsigned int blue) {
    // analogWrite(redPin1, red);
    // analogWrite(greenPin1, green);
    // analogWrite(bluePin1, blue);
    analogWrite(redPin2, red);
    analogWrite(greenPin2, green);
    analogWrite(bluePin2, blue);
}

void setup() {
    Serial.begin(57600);

    // Start off with the LED off.
    //setColorRgb(0, 0, 0);

    setColorRgb(0, 0, 0);
}

void loop() {
    // Start off with blue.
    unsigned int rgbColor[3] = { 0, 0, 255};

    // Repeat forever
    while(1) {
        // Choose the colours to increment and decrement (fade from blue > green > red)
        for (int decColour = 2; decColour >= 0; decColour -= 1) {
            int incColour = decColour == 0 ? 2 : decColour - 1;

            // Cross-fade the two colours.
            for(int i = 0; i < 255; i += 1) {
                // Increase delay when on a solid color so it seems smoother (otherwise it will almost immediately change color)
                delay(SPEED);
                if (rgbColor[0] == 255 || rgbColor[1] == 255 || rgbColor[2] == 255) {
                    delay(SPEED * 2);
                }

                rgbColor[decColour] -= 1;
                rgbColor[incColour] += 1;

                setColorRgb(rgbColor[0], rgbColor[1], rgbColor[2]);
                Serial.print("r: ");
                Serial.print(rgbColor[0]);
                Serial.print(" g: ");
                Serial.print(rgbColor[1]);
                Serial.print(" b: ");
                Serial.println(rgbColor[2]);
            }
        }
    }
}