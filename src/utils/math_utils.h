#pragma once

#include <Arduino.h>

namespace MathUtils {

// Standard linear interpolation
constexpr float lerp(float a, float b, float t) { return a + t * (b - a); }

// Maps a value from range [a, b] to [0.0, 1.0], clamped
inline float inverseLerpClamped(float a, float b, float value) {
  if (a == b) return 0.0f;
  return constrain((value - a) / (b - a), 0.0f, 1.0f);
}

}