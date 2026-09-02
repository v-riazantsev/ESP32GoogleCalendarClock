#pragma once

#include <stdint.h>

#include <string>

struct Event {
  std::string id;

  time_t startTimestamp;
  time_t endTimestamp;

  std::string summary;
  std::string description;

  uint32_t color;
};