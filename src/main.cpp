#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <WebSocketsClient.h>

#include "../credentials.h"
#include "dcf77.h"

DCF77Clock dcf77;

#define NUM_LEDS 60
#define HOUR_LEDS 24
#define HOUR_OFFSET 60
#define TOTAL_LEDS 84

#define HUE_GROUP1 145
#define HUE_GROUP2 50

// ***************************
// function header definitions
// ***************************

void drawHour();
void drawPixel(int pos, int value);

// some helper functions, see implementation at end of the file
void nblendU8TowardU8(uint8_t& cur, const uint8_t target, uint8_t amount);
CHSV fadeTowardColor(CHSV& cur, const CHSV& target, uint8_t amount);
CRGB sqrtBlend(CRGB a, CRGB b, uint8_t t);
CHSV sqrtBlend(CHSV a, CHSV b, uint8_t t);

/*
 *  we use 2 arrays:
 *  - "leds" is the normal array used by FastLED
 *  - "ledColors" is holding the current colors as HSV values for better
 * manipulation
 *
 *  sync from ledColors -> leds is being done at the end the drawPixel call
 */
CRGB leds[TOTAL_LEDS];
CHSV ledColors[NUM_LEDS];

WiFiClient wifi;
WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);
    } break;
    case WStype_TEXT: {
      StaticJsonDocument<200> doc;
      deserializeJson(doc, payload);

      if (doc["Type"] == 1) {
        int pulseLength = doc["Pulse"].as<int>();
        int pause = doc["Pause"].as<int>();
        dcf77.handlePulse(pulseLength, pause);
      }

      break;
    }
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    case WStype_PING:
      Serial.printf("[WSc] get ping\n");
      break;
    case WStype_PONG:
      Serial.printf("[WSc] get pong\n");
      break;
    case WStype_ERROR:
      Serial.printf("[WSc] got Error\n");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  FastLED.addLeds<WS2812B, D4, GRB>(leds, TOTAL_LEDS);

  delay(100);
  FastLED.clear();
  FastLED.show();

  // setup wifi and show startup animation
  WiFi.hostname("DCFClock");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int connectionLed = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    leds[connectionLed] = CRGB::Red;

    fadeToBlackBy(leds, NUM_LEDS, 20);
    connectionLed = (connectionLed + 1) % NUM_LEDS;
    FastLED.show();
    FastLED.delay(20);
  }
  WiFi.hostname("DCFClock");

  ArduinoOTA.begin();

  Serial.println(" connected!");

  // setup websocket
  webSocket.beginSSL("www.dcf77logs.de", 443, "/ajax/liveview");
  webSocket.setReconnectInterval(5000);
  webSocket.onEvent(webSocketEvent);

  // initialize the shadow-led array
  for (int i = 0; i < NUM_LEDS; i++) {
    ledColors[i] = CHSV(0, 0, 0);
  }
}

void loop() {
  webSocket.loop();
  ArduinoOTA.handle();

  for (int i = 0; i < NUM_LEDS; i++) {
    drawPixel(i, dcf77.getBit(i));
  }
  drawHour();

  FastLED.show();
  FastLED.delay(10);
}

/*
 * calculate and set the color (hue, saturation and brightness)
 * based on the given position (0 to 60) and the value (0 or 1)
 * @see https://de.wikipedia.org/wiki/DCF77 for pixels and meanings
 */
void drawPixel(int pos, int value) {
  /*
   * calculate the hsv values depending on the position
   */

  int hue = 0;
  int sat = 255;
  int brightness = 0;

  // set the base brightness for on / off pixels
  if (value == 0) {
    brightness = 50;
  } else if (value == 1) {
    brightness = 150;
  }

  if (pos <= 20) {  // start bit, weather/warning informations and shift-second
                    // / timezone information; currently ignored
    hue = HUE_GROUP1;
  } else if (pos >= 21 && pos <= 24) {  // 1-part of minutes
    hue = HUE_GROUP2;
  } else if (pos >= 25 && pos <= 27) {  // 10-part of minutes
    hue = HUE_GROUP1;
  }

  else if (pos == 28) {  // parity of minutes
    hue = value == dcf77.parity(21, 27) ? 96 : 0;
  }

  else if (pos >= 29 && pos <= 32) {  // 1-part of hours
    hue = HUE_GROUP2;
  } else if (pos >= 33 && pos <= 34) {  // 10-part of hours
    hue = HUE_GROUP1;
  }

  else if (pos == 35) {  // parity of hours
    hue = value == dcf77.parity(29, 34) ? 96 : 0;
  }

  else if (pos >= 36 && pos <= 39) {  // 1-part of day
    hue = HUE_GROUP2;
  } else if (pos >= 40 && pos <= 41) {  // 10-part of day
    hue = HUE_GROUP1;
  }

  else if (pos >= 42 && pos <= 44) {  // weekday
    hue = HUE_GROUP2;
  }

  else if (pos >= 45 && pos <= 48) {  // 1-part of month
    hue = HUE_GROUP1;
  } else if (pos >= 49 && pos <= 49) {  // 10-part of month
    hue = HUE_GROUP2;
  }

  else if (pos >= 50 && pos <= 53) {  // 1-part of year
    hue = HUE_GROUP1;
  }

  else if (pos >= 54 && pos <= 57) {  // 10-part of year
    hue = HUE_GROUP2;
  }

  else if (pos == 58) {  // parity bit for date
    hue = value == dcf77.parity(36, 57) ? 96 : 0;
  }

  else if (pos == 59) {  // shift-second - not used
    brightness = 0;
  }

  else {  // default, should not appear
    brightness /= 2;
    hue = (310 / 360.) * 255;
    sat = 255;
  }

  /*
   * overwrite values for current and last position
   */

  if (pos == dcf77.getPosition()) {  // current received bit
    if (value == 0) {                // if value is 0 - blend to darker color
      ledColors[pos] = sqrtBlend(ledColors[pos], CHSV(hue, sat, 15), 10);
    } else {  // if value is 1 - blend to lighter color
      ledColors[pos] = sqrtBlend(ledColors[pos], CHSV(hue, sat, 250), 2);
    }
  } else if (pos ==
             dcf77.getPosition() -
                 1) {  // last bit, fade it to default set color/brightness
    ledColors[pos] = sqrtBlend(ledColors[pos], CHSV(hue, sat, brightness), 120);
  } else {  // set to calculated value
    ledColors[pos] = CHSV(hue, sat, brightness);
  }

  // sync the hsv value from ledColors to leds array
  leds[pos] = ledColors[pos];

  // overwrite the current minute with bright white color
  if (pos == dcf77.getRealMinutes()) {
    leds[pos] = CHSV(0, 0, 200);
  }
}

/*
 * draw the current (real) hour to the inner circle
 * and the base 'clock' indicators
 */
void drawHour() {
  int realHour = dcf77.getRealHours();
  int realMinutes = dcf77.getRealMinutes();

  // if minutes are greater than 20, set the led between the hours on
  int led = (realHour * 2) % 24;
  if (realMinutes > 20) {
    led++;
  }
  // if minutes are greater than 40, set the led for the next hour to on
  if (realMinutes > 40) {
    led++;
  }

  // draw the clock indicators
  for (int i = 0; i <= HOUR_LEDS; i += 2) {
    if (i != led) {
      leds[i + HOUR_OFFSET] = CHSV(145, 255, 50);
    }
  }

  // slowly fade down all clock-leds for transition
  for (int i = HOUR_OFFSET; i <= HOUR_LEDS + HOUR_OFFSET; i++) {
    leds[i].fadeToBlackBy(10);
  }

  // set the led for the realHour
  if (realHour != -1) {
    leds[led + HOUR_OFFSET] = CHSV(255, 0, 200);
  }
}

// some helper functions for color fading / blending

void nblendU8TowardU8(uint8_t& cur, const uint8_t target, uint8_t amount) {
  if (cur == target) return;

  if (cur < target) {
    uint8_t delta = target - cur;
    delta = scale8_video(delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video(delta, amount);
    cur -= delta;
  }
}

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CHSV fadeTowardColor(CHSV& cur, const CHSV& target, uint8_t amount) {
  nblendU8TowardU8(cur.hue, target.hue, amount);
  nblendU8TowardU8(cur.sat, target.sat, amount);
  nblendU8TowardU8(cur.val, target.val, amount);
  return cur;
}

CRGB sqrtBlend(CRGB a, CRGB b, uint8_t t) {
  CRGB rgb;

  rgb.r = sqrt16(scale16by8(a.r * a.r, (255 - t)) + scale16by8(b.r * b.r, t));
  rgb.g = sqrt16(scale16by8(a.g * a.g, (255 - t)) + scale16by8(b.g * b.g, t));
  rgb.b = sqrt16(scale16by8(a.b * a.b, (255 - t)) + scale16by8(b.b * b.b, t));

  return rgb;
}

CHSV sqrtBlend(CHSV a, CHSV b, uint8_t t) {
  CHSV hsv;

  hsv.h = sqrt16(scale16by8(a.h * a.h, (255 - t)) + scale16by8(b.h * b.h, t));
  hsv.s = sqrt16(scale16by8(a.s * a.s, (255 - t)) + scale16by8(b.s * b.s, t));
  hsv.v = sqrt16(scale16by8(a.v * a.v, (255 - t)) + scale16by8(b.v * b.v, t));

  return hsv;
}
