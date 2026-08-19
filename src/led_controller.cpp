#include "led_controller.h"

LedController::LedController(uint16_t numPixels, uint8_t pin, neoPixelType type)
    : _strip(numPixels, pin, type), MAX_LEDS(numPixels) {
  _buf = new uint32_t[MAX_LEDS * 24];
}

void LedController::begin(uint8_t brightness) {
  _strip.begin();
  _strip.setBrightness(brightness);
  clear();
}

void LedController::clear() {
  for (uint16_t i = 0; i < MAX_LEDS; i++) _buf[i] = 0;
}

void LedController::addRangePct(float startPct, float endPct, uint32_t color) {
  // Keep the requested range inside the valid 0.0 to 1.0 range.
  startPct = constrain(startPct, 0.0f, 1.0f);
  endPct = constrain(endPct, 0.0f, 1.0f);

  // Nothing to paint if the range has no length.
  if (endPct <= startPct) return;

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

void LedController::show(uint currentTimestamp, float fadeDistance) {
  int offset = currentTimestamp;
  for (uint16_t i = 0; i < MAX_LEDS; i++) _strip.setPixelColor(i, _buf[i]);
  _strip.show();
}
