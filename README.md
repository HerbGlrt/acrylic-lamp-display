# Acrylic Lamp Photo Display

This project is a custom acrylic heart lamp made as a Christmas gift. It combines a laser-cut acrylic heart (design made in LightBurn), a small TFT display, a column-driven WS2812 LED edge-lighting system, and external image storage on a micro SD card. The result is a portable, battery-powered acrylic lamp that shows photos on the display and uses the image colors to drive the LED lighting for a beautiful synchronized effect.

## Project motivation

I built this as a Christmas present for my girlfriend. I was inspired by commercially available acrylic lamps but wanted a personal touch that highlights our best memories. I designed a custom heart-shaped acrylic panel in LightBurn with space to mount the TFT display and LEDs, then added features for portability and easy image updates (SD card) and powered it with a rechargeable 9V battery.

## Highlights

- Custom heart acrylic design with slots for a 1.8" TFT display and edge lighting.
- 1.8" 128×160 TFT shows photos from an SD card in uncompressed BMP format.
- Eight WS2812 (5050) RGB LED strips provide edge lighting. Each LED is driven by the average color of a corresponding image column for a pleasing, image-aware glow.
- Powered by a rechargeable 9V battery for portability; controlled by an Arduino UNO.
- Decorative rhinestones added to the heart corners for extra sparkle.

## Final result
## Final result

![Lamp 1](https://github.com/HerbGlrt/acrylic-lamp-display/blob/main/Images/IMG_0192.HEIC?raw=1)
![Lamp 2](https://github.com/HerbGlrt/acrylic-lamp-display/blob/main/Images/IMG_0207.HEIC?raw=1)
![Lamp 3](https://github.com/HerbGlrt/acrylic-lamp-display/blob/main/Images/IMG_0189.HEIC?raw=1)
![Lamp 4](https://github.com/HerbGlrt/acrylic-lamp-display/blob/main/Images/IMG_0204.HEIC?raw=1)
![Lamp 5](https://github.com/HerbGlrt/acrylic-lamp-display/blob/main/Images/IMG_0211.HEIC?raw=1)
![Lamp 6](https://github.com/HerbGlrt/acrylic-lamp-display/blob/main/Images/IMG_0212.HEIC?raw=1)

## Bill of materials

- Arduino UNO
- Rechargeable 9V battery
- WS2812 5050 RGB LED strip (8 LEDs used)
- 1.8" TFT LCD (128×160) (example: ST7735 compatible)
- Micro SD card module
- Micro SD card (FAT32)
- Jumpers, connectors, and a small project box / case
- Laser-cut acrylic heart (LightBurn files are in the `LightBurn/` folder)

## How it works (high level)

1. The Arduino reads BMP image files from the micro SD card.
2. The TFT display renders the current BMP file to the screen (using an ST7735-style driver library).
3. The same image is analyzed column-by-column; for each column the Arduino calculates the average RGB color and maps that color to one of the 8 WS2812 LEDs. This produces synchronized ambient lighting that follows the image colors.
4. Power is supplied by a rechargeable 9V battery and the whole assembly is mounted inside a small case attached to the acrylic heart.

## Image preparation (required for the SD card)

To ensure the Arduino + TFT can display the images, images must be prepared in a specific way. I used GIMP to convert and prepare each photo. Required image properties:

- Format: BMP (Windows bitmap)
- Color depth: 24-bit RGB (no alpha)
- Compression: none (no RLE)
- Resolution: equal to or smaller than the display (for a 1.8" 128×160 ST7735 use 160×128 — width × height as expected by your library)
- Orientation: top-to-bottom
- Filename: short 8.3 names (e.g., IMG01.BMP)
- Location: root of the SD card (FAT32 formatted)

GIMP export steps used:

1. Open the image in GIMP: File → Open
2. Resize: Image → Scale Image… set to 160×128 (or your display resolution)
3. Export: File → Export As…, choose "Windows BMP image" and set:
	- 24 bits
	- No RLE compression
	- Compatible with Windows
4. Save with a short filename like `IMG01.BMP` and copy to the SD card root.

Why these constraints? The Arduino libraries used for SD + TFT display typically expect raw BMP byte data. They don't handle compressed formats (JPEG, PNG) nor different bit depths.

## Image-to-LED mapping

I split each image into columns equal to the number of LEDs (8). For each column the firmware computes the average RGB value of the pixels in that column, converts it to the appropriate color values, and writes that color to the corresponding WS2812 LED. This calculation is done in real time in the firmware so new images copied to the SD card automatically produce matching LED lighting.

## Wiring and software notes

- TFT: connect SPI pins (SCK, MOSI, CS, DC, RST) + 3.3V power (follow your display module docs)
- SD module: SPI lines (can share SPI bus with TFT if CS lines are separate) + 5V/3.3V as appropriate
- WS2812 LEDs: single data line from an Arduino digital pin, common ground, and 5V supply (if using battery, ensure reliable 5V regulator)
- Battery: 9V rechargeable to a 5V regulator (or power Arduino Vin if suitable). Ensure safe charging and proper voltage regulation—do not drive LEDs directly from raw battery without regulation.
- Use level-shifting where needed for WS2812 data line if the Arduino logic voltage and LED supply differ.

Common Arduino libraries useful for this project (examples):

- Adafruit_GFX
- Adafruit_ST7735 or other ST7735 driver
- SD (or SDFat)
- FastLED or Adafruit_NeoPixel (for WS2812 control)

Note: exact wiring and code depend on the libraries you pick. The repo includes an Arduino sketch (`Code.ino`) — review pin assignments and library includes there.

## Reproducing the project (quick steps)

1. Laser-cut the acrylic heart from the LightBurn files in `LightBurn/` (files include `.lbrn2` and `.dxf`). Leave a rectangular cutout for the 1.8" TFT.
2. Assemble Arduino, SD module, TFT, and WS2812 strip in the case. Wire SPI and data lines as per your pin mapping in `Code.ino`.
3. Prepare images (see "Image preparation") and copy BMP files to SD card root.
4. Insert SD, power up, and the Arduino should display images and light LEDs based on image colors.

## Files and folders in this repository

- `Code.ino` — Arduino sketch (main firmware). Review pin mapping and libraries in this file.
- `LightBurn/` — LightBurn project files and DXF exports for the acrylic heart design.
- `Images/` — Images of the final projetc.
- `README.md`
