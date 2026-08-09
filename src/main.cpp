#include <Arduino.h>

#include "ledController.h"

#define PIN_WS2812B 32
#define NUM_PIXELS 44

LedController strip(NUM_PIXELS, PIN_WS2812B);

void setup() {
  Serial.begin(9600);
  strip.begin(3);  // brightness
}

void loop() {
  strip.clear();

  strip.addRangePct(0.0f, 0.0833f, strip.Color(255, 0, 0));
  strip.addRangePct(0.0833f, 0.1666f, strip.Color(0, 255, 0));
  strip.addRangePct(0.1666f, 0.25f, strip.Color(255, 0, 0));
  strip.addRangePct(0.25f, 0.5f, strip.Color(0, 0, 255));
  strip.addRangePct(0.5f, 0.75f, strip.Color(100, 0, 0));
  strip.addRangePct(0.75f, 1.0f, strip.Color(255, 255, 255));

  strip.show(1000, 0.0f);

  delay(5000);
}
