#include "led_controller.h"

#include <cmath>

#include "config.h"

LedController::LedController(EventManager& eventManager,
                             TimeManager& timeManager)
    : _eventManager(eventManager),
      _timeManager(timeManager),
      MAX_LEDS(NUM_PIXELS),
      _taskHandle(nullptr),
      _lastRenderMs(0),
      _isRunning(false) {
  _buf = new CRGB[MAX_LEDS];

  FastLED.addLeds<WS2812B, PIN_WS2812B, GRB>(_buf, MAX_LEDS);

  // Non-linear gamma correction
  FastLED.setCorrection(TypicalLEDStrip);

  // Enables temporal dithering
  FastLED.setDither(BINARY_DITHER);

  FastLED.setBrightness(LED_DEFAULT_BRIGHTNESS);

  clear();
}

void LedController::startTask() {
  if (_taskHandle != nullptr) return;

  _isRunning = true;

  xTaskCreatePinnedToCore(
      [](void* param) {
        auto* controller = static_cast<LedController*>(param);
        controller->update();
      },
      "LedUpdateTask", 8192, this, 1, &_taskHandle, 1);
}

void LedController::update() {
  while (_isRunning) {
    uint32_t now = millis();
    uint32_t refreshInterval = refreshIntervals[_mode];

    if (_lastRenderMs == 0 || now - _lastRenderMs >= refreshInterval) {
      _lastRenderMs = now;

      clear();

      switch (_mode) {
        case LedMode::Events:
          fillEvents(_eventManager.getEvents(), _timeManager.now(), 3600 * 3);
          break;

        case LedMode::None:
          break;
      }

      std::lock_guard<std::mutex> lock(_mutex);
      uint8_t scaledBrightness = dim8_raw(_brightness);
      nscale8(_buf, MAX_LEDS, scaledBrightness);
    }

    // Refresh continuously inside the loop to drive FastLED's temporal
    // dithering
    FastLED.show();

    FastLED.delay(2);
  }

  showBlank();

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void LedController::setBrightness(uint8_t brightness) {
  std::lock_guard<std::mutex> lock(_mutex);
  _brightness = brightness;
}

void LedController::showBlank() {
  clear();
  FastLED.show();
}

void LedController::clear() { fill_solid(_buf, MAX_LEDS, CRGB::Black); }

template <typename Func>
void LedController::forEachLedInRange(float startPct, float endPct, Func&& fn) {
  if (endPct == startPct) return;

  float startPos = startPct * MAX_LEDS;
  float endPos = endPct * MAX_LEDS;

  float rangeLength = (startPos < endPos) ? (endPos - startPos)
                                          : (MAX_LEDS - startPos + endPos);

  uint16_t startLed = static_cast<uint16_t>(startPos);
  uint16_t endLed = static_cast<uint16_t>(std::ceil(endPos));

  for (uint16_t i = startLed, j = 0; i != endLed;
       i = (i >= MAX_LEDS) ? 0 : i + 1, j++) {
    float coveragePct = 1.0f;
    if (i == startLed) coveragePct -= (startPos - startLed);
    if (i == endLed - 1) coveragePct -= ((endLed - 1) + 1.0f - endPos);

    coveragePct = std::max(0.0f, coveragePct);
    float progress =
        (rangeLength > 0.0f) ? static_cast<float>(j) / rangeLength : 0.0f;

    fn(i, coveragePct, progress);
  }
}

void LedController::addRangePct(float startPct, float endPct, CRGB paintColor) {
  forEachLedInRange(
      startPct, endPct,
      [this, paintColor](uint16_t i, float coveragePct, float) {
        // FastLED dim8 curve preserves low-end color resolution
        uint8_t scale = dim8_raw(static_cast<uint8_t>(coveragePct * 255.0f));

        CRGB scaledPaint = paintColor;
        scaledPaint.nscale8(scale);

        bool isBlack = (_buf[i].r == 0 && _buf[i].g == 0 && _buf[i].b == 0);
        uint8_t blendAmount = isBlack ? scale : 128;

        nblend(_buf[i], scaledPaint, blendAmount);
      });
}

void LedController::fadeRangePct(float startPct, float endPct, CRGB fadeColor,
                                 bool inverse) {
  forEachLedInRange(startPct, endPct,
                    [this, fadeColor, inverse](uint16_t i, float coveragePct,
                                               float progress) {
                      float fadeValue = 1.0f - (progress * std::sqrt(progress));

                      if (inverse) fadeValue = 1.0f - fadeValue;

                      uint8_t blendAmount = static_cast<uint8_t>(constrain(
                          fadeValue * coveragePct * 255.0f, 0.0f, 255.0f));

                      nblend(_buf[i], fadeColor, blendAmount);
                    });
}

void LedController::fillEvents(const std::vector<Event>& events,
                               time_t currentTimestamp, uint32_t fadeDistance) {
  int32_t displayRangeOffsetStart =
      -(int32_t)fadeDistance + (12 * 3600 / MAX_LEDS);
  int32_t displayRangeOffsetEnd = 12 * 3600 - (int32_t)fadeDistance;

  for (const Event& event : events) {
    int32_t startOffset = event.startTimestamp - currentTimestamp;
    int32_t endOffset = event.endTimestamp - currentTimestamp;

    if (endOffset < -(int32_t)fadeDistance &&
        startOffset < -(int32_t)fadeDistance)
      continue;

    if (startOffset > 12 * 3600 - (int32_t)fadeDistance) continue;

    // Clamp offset values to the display range.
    startOffset =
        constrain(startOffset, displayRangeOffsetStart, displayRangeOffsetEnd);
    endOffset =
        constrain(endOffset, displayRangeOffsetStart, displayRangeOffsetEnd);

    float startPct = _timeManager.clock12hPct(currentTimestamp + startOffset);
    float endPct = _timeManager.clock12hPct(currentTimestamp + endOffset);

    addRangePct(startPct, endPct, event.color);
  }

  if (fadeDistance != 0 && !events.empty()) {
    float fadeStartPct =
        _timeManager.clock12hPct(currentTimestamp - fadeDistance);
    float fadeEndPct = _timeManager.clock12hPct(currentTimestamp);

    fadeRangePct(fadeStartPct, fadeEndPct, CRGB::Black);
  }
}