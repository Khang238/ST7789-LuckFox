# Simple Graphics Library for SPI ST7789 Display

## Setup:
Requirements: Have the Luckfox SDK and add the Luckfox SDK toolchain to your PATH, for example:
```
echo 'export PATH=[directory location]/luckfox-pico/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin:$PATH' >> ~/.bashrc
```
Build with `make`, `example.c` is already available for you to test the library's features.
Make sure you have enabled SPI device (`/dev/spidev0.0`, use `luckfox-config` on Luckfox to enable SPI if not already) for everything to work.

## Pinout:
<img width="1107" height="461" alt="image" src="https://github.com/user-attachments/assets/e02b81a7-5bd8-4669-b437-95e8c0e97bfd" />

| Display Pin      | Pico Pin       |
|------------------|----------------|
| VCC              | 3.3V           |
| GND              | GND            |
| SCL (SCK)        | 49 / SPI0_CLK  |
| SDA (MOSI)       | 50 / SPI0_MOSI |
| RES (Reset)      | 56             |
| DC (Data/CMD)    | 57             |
| CS (Chip Select) | 48 / SPI0_CS0  |
| BL (Backlight)   | 54 / PMW10_M1  |

## Note:
- I used Claude AI to code **THE ENTIRE** library, so please don't expect too much from it.
- Any good feedback for this library (except kaboom) is welcome. The library is also unlicensed; you can upgrade it and use it as your own if needed.
