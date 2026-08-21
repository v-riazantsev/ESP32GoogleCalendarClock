#pragma once

#include <stdint.h>

#include <string>

struct Event {
  std::string id;

  uint64_t startTimestamp;
  uint64_t endTimestamp;

  std::string summary;
  std::string description;

  uint32_t color;
};