#pragma once

#include <Arduino.h>

#include <map>
#include <vector>

struct BuzzerTone {
  uint16_t frequency;
  uint16_t durationMs;
};

enum class BuzzerPreset {
  EventStart,
  EventEnd,
  Acknowledge,
};

const std::map<BuzzerPreset, std::vector<BuzzerTone>> BuzzerPresets = {
    {
        BuzzerPreset::EventStart,
        {
            {2000, 200},
            {0, 50},
            {2500, 200},
            {0, 50},
            {3000, 200},
        },
    },
    {
        BuzzerPreset::EventEnd,
        {
            {3000, 200},
            {0, 50},
            {2500, 200},
            {0, 50},
            {2000, 200},
        },
    },
    {
        BuzzerPreset::Acknowledge,
        {
            {2000, 100},
            {0, 50},
            {2000, 100},
        },
    },
};