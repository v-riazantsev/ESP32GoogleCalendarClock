#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LedController {
 public:
  uint16_t MAX_LEDS;
  LedController(uint16_t numPixels, uint8_t pin,
                neoPixelType type = (NEO_GRB + NEO_KHZ800));

  void begin(uint8_t brightness);
  void clear();  // clears framebuffer
  void addRangePct(float startPct, float endPct, uint32_t color);  // 0..100 %
  void show(uint currentTimestamp, float fadeDistance = 0.0f);

  uint32_t Color(uint8_t r, uint8_t g, uint8_t b) const {
    return _strip.Color(r, g, b);
  }

 private:
  Adafruit_NeoPixel _strip;
  uint32_t* _buf;
};
