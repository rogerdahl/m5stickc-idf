# M5StickC-IDF

ESP-IDF component to work with M5StickC. Works without including the Arduino framework.

---

## Current features:

* Initializes power management IC AXP192
* Initializes display ST7735
* Set display backlight brightness
* Display API thanks to Loboris ESP32_TFT_library

## Usage

Clone repository in your *components* folder in your project and include *m5stickc.h* in your source code.

Call **m5stickc_init()** to initialize your M5StickC.

Call **m5stickc_set_display_brightness(uint8_t brightness)** to set the backlight brightness on the display. Brightness range between 0x00 and 0x0F.

## Display driver based on Loboris ESP32_TFT_library

Includes a modified version of [ESP32_TFT_library](https://github.com/loboris/ESP32_TFT_library) by Boris Lovosevic. Check his GitHub for help on how to use the display.

## Contributing 

If you find it useful and add a feature, find a bug, improve documentation or anything else, I encourage you to make a pull request.

This is a work in progress. It will be updated with new features as they are needed, and we could definitely use your help!

Licensed under the MIT License.
