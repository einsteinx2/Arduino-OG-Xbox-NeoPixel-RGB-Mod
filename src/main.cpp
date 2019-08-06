#include <Arduino.h>

#define SPEED 4

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
    // Start off with the LED off.
    //setColorRgb(0, 0, 0);

    setColorRgb(0, 0, 0);
}

void loop() {
    unsigned int rgbColour[3];

    // Start off with red.
    rgbColour[0] = 255;
    rgbColour[1] = 0;
    rgbColour[2] = 0;  

    // Choose the colours to increment and decrement.
    for (int decColour = 0; decColour < 3; decColour += 1) {
        int incColour = decColour == 2 ? 0 : decColour + 1;

        // cross-fade the two colours.
        for(int i = 0; i < 255 / SPEED; i += SPEED) {
            rgbColour[decColour] -= SPEED;
            rgbColour[incColour] += SPEED;

            setColorRgb(rgbColour[0], rgbColour[1], rgbColour[2]);
            Serial.print("r: ");
            Serial.print(rgbColour[0]);
            Serial.print(" g: ");
            Serial.print(rgbColour[1]);
            Serial.print(" b: ");
            Serial.println(rgbColour[2]);
            delay(100);
        }
    }
    delay(100);
}