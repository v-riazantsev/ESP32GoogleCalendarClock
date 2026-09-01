#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include <vector>

#include "event.h"
#include "event_manager.h"

enum class LedMode { None, Events };

class LedController {
 public:
  uint16_t MAX_LEDS;
  LedController(uint16_t numPixels, uint8_t pin, EventManager& eventManager);

  void setBrightness(uint8_t brightness);
  void switchMode(LedMode mode);

 private:
  Adafruit_NeoPixel _strip;
  LedMode _mode = LedMode::None;
  EventManager& _eventManager;
  uint32_t* _buf;
  bool _isRunning;
  TaskHandle_t _taskHandle;

  void clear();    // clears framebuffer
  void refresh();  // refreshes the strip with the framebuffer
  void update();
  void addRangePct(float startPct, float endPct, uint32_t color);  // 0..100 %
  void fillEvents(const std::vector<Event>& events, uint32_t currentTimestamp,
                  uint32_t fadeDistance);
  float wrap(float val, float length) {
    if (val == length) return length;
    float result = std::fmod(val, length);
    return (result < 0.0f) ? (result + length) : result;
  }
  void showBlank() {
    clear();
    refresh();
  };
};
