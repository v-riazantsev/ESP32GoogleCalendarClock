#pragma once
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include <map>
#include <vector>

#include "event.h"
#include "event_manager.h"
#include "time_manager.h"

struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  bool isBlack() const { return r == 0 && g == 0 && b == 0; }
  RGB() : r(0), g(0), b(0) {}
  RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
  RGB(uint32_t color) {
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;
  }
};

enum class LedMode { None, Events };

class LedController {
 public:
  uint16_t MAX_LEDS;
  LedController(uint16_t numPixels, uint8_t pin, EventManager& eventManager,
                TimeManager& timeManager);

  void setBrightness(uint8_t brightness);
  void startTask();
  void switchMode(LedMode mode) {
    _mode = mode;
    _lastRenderMs = 0;
  };

 private:
  Adafruit_NeoPixel _strip;
  LedMode _mode = LedMode::None;
  EventManager& _eventManager;
  TimeManager& _timeManager;
  RGB* _buf;
  bool _isRunning;
  TaskHandle_t _taskHandle;
  uint32_t _lastRenderMs;

  std::map<LedMode, uint32_t> refreshIntervals = {{LedMode::Events, 1000},
                                                  {LedMode::None, 60000}};

  void clear();    // clears framebuffer
  void refresh();  // refreshes the strip with the framebuffer
  void update();
  void addRangePct(float startPct, float endPct, RGB paintColor);  // 0..100 %
  void fadeRangePct(float startPct, float endPct, RGB fadeColor,
                    float startValue = 0.0f,
                    float endValue = 1.0f);  // 0..100 %
  void fillEvents(const std::vector<Event>& events, time_t currentTimestamp,
                  uint32_t fadeDistance);
  template <typename Func>
  void forEachLedInRange(float startPct, float endPct,
                         Func&& callback);  // 0..100 %
  float lerpFast(float a, float b, float t) { return a + t * (b - a); }
  float inverseLerpClamped(float a, float b, float value) {
    if (a == b) return 0.0f;
    return constrain((value - a) / (b - a), 0.0f, 1.0f);
  }
  RGB lerpColor(RGB a, RGB b, float t) {
    return {static_cast<uint8_t>(lerpFast(a.r, b.r, t)),
            static_cast<uint8_t>(lerpFast(a.g, b.g, t)),
            static_cast<uint8_t>(lerpFast(a.b, b.b, t))};
  }
  void showBlank() {
    clear();
    refresh();
  };
};
