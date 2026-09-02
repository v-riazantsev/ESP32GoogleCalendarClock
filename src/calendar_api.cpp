#include "calendar_api.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "secrets.h"

bool CalendarApi::fetchEvents(std::vector<Event>& events) {
  // Check if Wi-Fi is connected before making the HTTP request.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi not connected. Cannot fetch events.");
    return false;
  }

  HTTPClient http;

  String url = String(GOOGLE_APP_URL) + String(GOOGLE_APP_TOKEN);

  // Serial.println("Requesting:");
  // Serial.println(url);

  http.begin(url);

  // Follow redirects from Google Apps Script.
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // Perform the request once.
  int httpCode = http.GET();

  // Serial.print("HTTP status: ");
  // Serial.println(httpCode);

  if (httpCode != HTTP_CODE_OK) {
    Serial.print("HTTP error: ");
    Serial.println(http.errorToString(httpCode));

    http.end();
    return false;
  }

  String response = http.getString();

  // Serial.println("API Response:");
  // Serial.println(response);

  http.end();

  // Clear the events vector.
  events.clear();

  // Parse the JSON response.
  JsonDocument document;

  DeserializationError error = deserializeJson(document, response);

  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.f_str());
    return false;
  }

  // Convert the JSON array to a vector of Event objects.
  for (JsonObject jsonEvent : document.as<JsonArray>()) {
    Event event;

    event.id = jsonEvent["id"].as<std::string>();

    event.startTimestamp = jsonEvent["startTimestamp"];
    event.endTimestamp = jsonEvent["endTimestamp"];

    event.summary = jsonEvent["summary"].as<std::string>();
    event.description = jsonEvent["description"].as<std::string>();

    event.color = jsonEvent["color"];

    events.push_back(event);
  }
  return true;
}