#pragma once

#include <vector>

#include "event.h"

class CalendarApi {
 public:
  bool fetchEvents(std::vector<Event>& events);
};