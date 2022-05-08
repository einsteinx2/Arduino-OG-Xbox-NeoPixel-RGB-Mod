Arduino project to add NeoPixels to the power button and controller ports of an OG Xbox and have it detect when the system displays error lights (flashing red, flashing green, etc), but show a rainbow effect (or whatever you want to program) when in solid green mode.

WIP video here: https://www.youtube.com/watch?v=jr9lz8rE8Vg

This project relies on a couple of small shared Platform.IO libraries that I wrote to share general Arduino and LED strip code between projects. In order to compile successfully, make sure that this project is located in your `PlatformIO/Projects/` folder and then clone the following repos into your `PlatformIO/Libraries/` folder:
- https://github.com/einsteinx2/ArduinoUtils
- https://github.com/einsteinx2/LedStrip

Alternatively, if you'd like the project to be self contained, you can simply clone the library repos into the `lib` folder inside this project (and optionally remove the `lib_extra_dirs = ../../Libraries` bit from the `platformio.ini` file).

Will add more details later.

Licensed under GPLv3.