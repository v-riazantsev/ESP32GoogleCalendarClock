#include "led_controller.h"

#include <cmath>

LedController::LedController(uint16_t numPixels, uint8_t pin,
                             EventManager& eventManager,
                             TimeManager& timeManager)
    : _strip(numPixels, pin),
      _eventManager(eventManager),
      _timeManager(timeManager),
      MAX_LEDS(numPixels) {
  _buf = new RGB[MAX_LEDS];
  _strip.begin();
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

      refresh();
    }

    vTaskDelay(pdMS_TO_TICKS(32));
  }

  showBlank();

  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void LedController::refresh() {
  for (uint16_t i = 0; i < MAX_LEDS; i++) {
    _strip.setPixelColor(i, _buf[i].r, _buf[i].g, _buf[i].b);
  }

  _strip.show();
}

void LedController::setBrightness(uint8_t brightness) {
  _strip.setBrightness(brightness);
}

void LedController::clear() {
  for (uint16_t i = 0; i < MAX_LEDS; i++) _buf[i] = RGB();
}

// Reusable Helper: Encapsulates strip geometry and anti-aliased range iteration
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
    // Sub-pixel coverage calculation
    float coveragePct = 1.0f;
    if (i == startLed) coveragePct -= (startPos - startLed);
    if (i == endLed - 1) coveragePct -= ((endLed - 1) + 1.0f - endPos);

    coveragePct = std::max(0.0f, coveragePct);
    float progress =
        (rangeLength > 0.0f) ? static_cast<float>(j) / rangeLength : 0.0f;

    // Delegate pixel logic to caller
    fn(i, coveragePct, progress);
  }
}

void LedController::addRangePct(float startPct, float endPct, RGB paintColor) {
  forEachLedInRange(
      startPct, endPct,
      [this, paintColor](uint16_t i, float coveragePct, float) {
        RGB currentColor = _buf[i];
        float blendFactor = currentColor.isBlack() ? coveragePct : 0.5f;
        _buf[i] = lerpColor(currentColor, paintColor, blendFactor);
      });
}

void LedController::fadeRangePct(float startPct, float endPct, RGB fadeColor,
                                 float startValue, float endValue) {
  forEachLedInRange(
      startPct, endPct,
      [this, fadeColor, startValue, endValue](uint16_t i, float coveragePct,
                                              float progress) {
        RGB currentColor = _buf[i];
        float fadeValue =
            MathUtils::lerp(startValue, endValue, 1.0f - progress * progress);
        _buf[i] = lerpColor(currentColor, fadeColor, fadeValue * coveragePct);
      });
}

void LedController::fillEvents(const std::vector<Event>& events,
                               time_t currentTimestamp, uint32_t fadeDistance) {
  // The display range is from -fadeDistance + 1 LED to 12 hours in the future.
  int32_t displayRangeOffsetStart =
      -(int32_t)fadeDistance + (12 * 3600 / MAX_LEDS);
  int32_t displayRangeOffsetEnd = 12 * 3600 - (int32_t)fadeDistance;

  for (const Event& event : events) {
    // Still use UTC timestamps for determining
    // whether the event is relevant.
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

    // Convert event timestamps to LOCAL clock positions.
    float startPct = _timeManager.clock12hPct(currentTimestamp + startOffset);

    float endPct = _timeManager.clock12hPct(currentTimestamp + endOffset);

    addRangePct(startPct, endPct, event.color);
  }

  // Apply fade effect for events that are fading out.
  if (fadeDistance != 0 && events.size() > 0) {
    float fadeStartPct =
        _timeManager.clock12hPct(currentTimestamp - fadeDistance);
    float fadeEndPct = _timeManager.clock12hPct(currentTimestamp);

    fadeRangePct(fadeStartPct, fadeEndPct, 0x000000);
  }
}
