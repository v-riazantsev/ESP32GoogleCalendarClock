#include "led_controller.h"

LedController::LedController(uint16_t numPixels, uint8_t pin,
                             EventManager& eventManager)
    : _strip(numPixels, pin), _eventManager(eventManager), MAX_LEDS(numPixels) {
  _buf = new uint32_t[MAX_LEDS * 24];
  _strip.begin();
  clear();
}

void LedController::switchMode(LedMode mode) {
  if (_mode == mode) return;  // No change needed
  _mode = mode;

  if (mode == LedMode::None) {
    _isRunning = false;  // Signal the task to stop
    return;
  }

  _isRunning = true;
  xTaskCreatePinnedToCore(
      [](void* param) {
        LedController* controller = static_cast<LedController*>(param);
        controller->update();
      },
      "LedUpdateTask",  // Task name
      8192,             // Stack size
      this,             // Pass the object instance
      1,                // Task priority
      &_taskHandle,     // Task handle
      0                 // Core ID
  );
}

void LedController::update() {
  while (_isRunning) {
    clear();
    switch (_mode) {
      case LedMode::Events: {
        fillEvents(_eventManager.getEvents(),  // Current events
                   1788263686 - 3600 * 1.5f,   // Current timestamp
                   3600 * 3);  // Fade distance in seconds (3 hours)
      } break;
    }
    refresh();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  showBlank();
  _taskHandle = nullptr;
  vTaskDelete(nullptr);
}

void LedController::refresh() {
  for (uint16_t i = 0; i < MAX_LEDS; i++) _strip.setPixelColor(i, _buf[i]);
  _strip.show();
}

void LedController::setBrightness(uint8_t brightness) {
  _strip.setBrightness(brightness);
}

void LedController::clear() {
  for (uint16_t i = 0; i < MAX_LEDS; i++) _buf[i] = 0;
}

void LedController::addRangePct(float startPct, float endPct, uint32_t color) {
  Serial.print("Adding range: ");
  Serial.print(startPct);
  Serial.print(" to ");
  Serial.print(endPct);
  Serial.print(" with color: ");
  Serial.println(color, HEX);
  // Keep the requested range inside the valid 0.0 to 1.0 range.
  Serial.print("Before wrap: startPct = ");
  Serial.print(startPct);
  Serial.print(", endPct = ");
  Serial.println(endPct);
  startPct = wrap(startPct, 1.0f);
  endPct = wrap(endPct, 1.0f);
  Serial.print("After wrap: startPct = ");
  Serial.print(startPct);
  Serial.print(", endPct = ");
  Serial.println(endPct);
  // Nothing to paint if the range has no length.
  if (endPct < startPct) {
    addRangePct(startPct, 1.0f, color);
    addRangePct(0.0f, endPct, color);
    return;
  }

  // Extract the red, green, and blue components from the color.
  uint8_t paintRed = (color >> 16) & 255;
  uint8_t paintGreen = (color >> 8) & 255;
  uint8_t paintBlue = color & 255;

  // Calculate how much of the normalized strip is occupied by one LED.
  float ledLength = 1.0f / (float)MAX_LEDS;

  // Process every LED and find how much of it overlaps the requested range.
  for (uint16_t ledIndex = 0; ledIndex < MAX_LEDS; ledIndex++) {
    // Calculate the normalized start and end position of this LED.
    float ledStart = ledIndex * ledLength;
    float ledEnd = ledStart + ledLength;

    // Find the overlapping
    float overlapStart = (ledStart > startPct) ? ledStart : startPct;
    float overlapEnd = (ledEnd < endPct) ? ledEnd : endPct;

    // Calculate how much of this LED is covered by the requested range.
    float overlapLength = overlapEnd - overlapStart;

    // Skip this LED if the requested range does not touch it.
    if (overlapLength <= 0) continue;

    // Convert the overlap into a value between 0.0 and 1.0.
    float coverage = overlapLength / ledLength;

    // Read the LED's current RGB color.
    uint32_t currentColor = _buf[ledIndex];

    // Extract the current red, green, and blue components.
    int r = (currentColor >> 16) & 255;
    int g = (currentColor >> 8) & 255;
    int b = currentColor & 255;

    // Add the requested color based on how much of the LED is covered.
    r += paintRed * coverage + 0.5f;
    g += paintGreen * coverage + 0.5f;
    b += paintBlue * coverage + 0.5f;

    // Limit the RGB values to the valid 0 to 255 range.
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;

    // Store the updated RGB color back into the LED buffer.
    _buf[ledIndex] = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
  }
}

void LedController::fillEvents(const std::vector<Event>& events,
                               uint32_t currentTimestamp,
                               uint32_t fadeDistance) {
  clear();
  Serial.print("Filling ");
  Serial.print(events.size());
  Serial.print(" events at timestamp: ");
  Serial.println(currentTimestamp);
  for (const Event& event : events) {
    if (event.endTimestamp < currentTimestamp - fadeDistance)
      continue;  // Skip past events
    if (event.startTimestamp > currentTimestamp + 3600 * 12 - fadeDistance)
      continue;  // Skip events that are too far in the future

    float startPct =
        (float)((int32_t)(event.startTimestamp - currentTimestamp)) /
        (3600.0f * 12);
    float endPct = (float)((int32_t)(event.endTimestamp - currentTimestamp)) /
                   (3600.0f * 12);

    addRangePct(startPct, endPct, event.color);
  }
  refresh();
}
