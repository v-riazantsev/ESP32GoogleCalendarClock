#pragma once

#include <vector>

#include "event.h"

class CalendarApi {
 public:
  std::vector<Event> fetchEvents();
};