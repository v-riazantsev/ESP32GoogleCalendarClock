// Configuration
const API_TOKEN = 'API_TOKEN';

const LOOK_BEFORE_H = 12;
const LOOK_AHEAD_H = 42;

// HTTP endpoint
function doGet(e) {
  // Check the API token before accessing the calendar.
  if (!e.parameter || e.parameter.token !== API_TOKEN) {
    return jsonResponse({
      error: 'Unauthorized'
    });
  }

  // Calculate the time window for events.
  const now = new Date();

  const lookBeforeMs = LOOK_BEFORE_H * 60 * 60 * 1000;
  const lookAheadMs = LOOK_AHEAD_H * 60 * 60 * 1000;

  const windowStart = new Date(now.getTime() - lookBeforeMs);
  const windowEnd = new Date(now.getTime() + lookAheadMs);

  // Get events from the default Google Calendar.
  const calendar = CalendarApp.getDefaultCalendar();
  const calendarEvents = calendar.getEvents(windowStart, windowEnd);

  // Get calendar default color as base fallback
  const defaultCalendarColor = googleColorToRgb(String(calendar.getColor()));

  // Convert Google Calendar events to small format.
  const events = calendarEvents.map(event => {
    const eventColorId = event.getColor();

    return {
      id: event.getId(),

      startTimestamp:
        Math.floor(event.getStartTime().getTime() / 1000),

      endTimestamp:
        Math.floor(event.getEndTime().getTime() / 1000),

      summary: event.getTitle(),

      description: event.getDescription(),

      // If event has no custom color (returns ""), fall back to calendar color
      color: eventColorId ? googleColorToRgb(String(eventColorId)) : defaultCalendarColor
    };
  });

  return jsonResponse(events);
}

// Google Calendar color mapping
function googleColorToRgb(colorId) {
  const colors = {
    '1':  0x7986CB, // Lavender
    '2':  0x33B679, // Sage
    '3':  0x8E24AA, // Grape
    '4':  0xE67C73, // Flamingo
    '5':  0xF6BF26, // Banana
    '6':  0xF4511E, // Tangerine
    '7':  0x039BE5, // Peacock
    '8':  0x616161, // Graphite
    '9':  0x3F51B5, // Blueberry
    '10': 0x0B8043, // Basil
    '11': 0xD50000, // Tomato
    '12': 0xAD1457, // Crimson / Deep Pink
    '13': 0xD81B60, // Raspberry
    '14': 0xE91E63, // Pink
    '15': 0xF4511E, // Coral
    '16': 0xFB8C00, // Orange
    '17': 0xFFB300, // Amber / Gold
    '18': 0xFDD835, // Yellow
    '19': 0xC0CA33, // Lime
    '20': 0x7CB342, // Light Green
    '21': 0x00897B, // Teal / Cyan Green
    '22': 0x00ACC1, // Sky / Cyan
    '23': 0x8D6E63, // Cocoa / Brown
    '24': 0x8E24AA  // Dark Purple
  };

  // Fallback to Google Blue (0x039BE5) instead of white if ID is unmapped
  return colors[colorId] || 0x039BE5;
}

// JSON response helper
function jsonResponse(data) {
  return ContentService
    .createTextOutput(JSON.stringify(data))
    .setMimeType(ContentService.MimeType.JSON);
}