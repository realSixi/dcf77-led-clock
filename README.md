# DCF77 LED Clock

A visualization for the DCF77 signal using 2 WS2812 / Neopixel LED Rings (60 and 24 LEDs). The DCF77 Signal is received via a Websocket Stream.


The intention is *not* to simple show the current time, but to show the single (bit) signals of the DCF77 signal.

Find more Information about DCF77 at Wikipedia: https://en.wikipedia.org/wiki/DCF77


![3D Printed Clock](./static/dcf77-clock.jpeg)

## Hardware

### 3D Print

Find the STL Files in the [3D-Parts](./3d-parts) Folder. 

- Printed with a Prusa MK3S+ with PETG Filament
- The "Shader" is printed with white PETG

![3D Printed Clock](./static/rendered_image.png)


### LEDs

There are multiple shops that deliver WS2812/Neopixel Rings (to Germany). Here is a short, uncomplete list. Just search for 'ws2812 ring' and you find a bunch of suppliers ;)

#### 60 LED Ring

- https://www.funduinoshop.com/WS2812-LEDs-mit-60-Pixeln
- https://www.berrybase.de/sensoren-module/led/ws2812-13-neopixel/ringe/neopixel-ring-mit-60-ws2812-5050-rgb-leds
- https://de.aliexpress.com/w/wholesale-ws2812-ring-60.html

*Watch out! Some sellers offer only 1/4th of the Ring! So read the descriptions with care.*

#### 24 LED Ring

- https://www.funduinoshop.com/epages/78096195.sf/de_DE/?ObjectPath=/Shops/78096195/Products/F-5-3
- https://www.berrybase.de/sensoren-module/led/ws2812-13-neopixel/ringe/neopixel-ring-mit-24-ws2812-5050-rgb-leds?c=2428


## Software

### Setup

1. Rename the `credentials_example.h` in the root folder to `credentials.h` and set your WiFi credentials.
2. Run with PlatformIO.

### Used Libraries / Tools

- PlatformIO http://platformio.org/
- FastLED: https://github.com/FastLED/FastLED
- WebSocket for Arduino: https://github.com/Links2004/arduinoWebSockets
- ArduinoJson: https://arduinojson.org/