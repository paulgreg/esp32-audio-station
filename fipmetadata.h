#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

char lastMetadata[LABEL_BUFFER_SIZE];

bool fetchRadioFranceMetadata(int metadataId, char* buffer, size_t bufferSize) {
  char url[128];
  snprintf(url, sizeof(url), "%s%d", RADIO_FRANCE_LIVEMETA_URL, metadataId);

  Serial.printf("Fetching metadata: %s\n", url);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(5000);
  int httpCode = http.GET();

  if (httpCode <= 0) {
    Serial.printf("[Metadata] GET failed: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  JSONVar json = JSON.parse(payload);
  if (JSON.typeof(json) == "undefined") {
    Serial.println("[Metadata] JSON parse failed");
    return false;
  }

  JSONVar levels = json["levels"];
  if (JSON.typeof(levels) == "undefined" || levels.length() == 0) {
    Serial.println("[Metadata] No levels");
    return false;
  }

  JSONVar level = levels[0];
  int position = (int)(double)level["position"];
  JSONVar items = level["items"];
  if (JSON.typeof(items) == "undefined" || items.length() == 0) {
    Serial.println("[Metadata] No items");
    return false;
  }

  JSONVar uidVar = items[position];
  if (JSON.typeof(uidVar) == "undefined") {
    Serial.println("[Metadata] No uid at position");
    return false;
  }

  const char* uid = (const char*)uidVar;
  JSONVar step = json["steps"][uid];
  if (JSON.typeof(step) == "undefined") {
    Serial.println("[Metadata] No step for uid");
    return false;
  }

  const char* title = (const char*)step["title"];
  const char* authors = (const char*)step["authors"];
  if (title == NULL || strlen(title) == 0) {
    Serial.println("[Metadata] No title");
    return false;
  }

  char formatted[LABEL_BUFFER_SIZE];
  if (authors != NULL && strlen(authors) > 0) {
    snprintf(formatted, sizeof(formatted), "%s - %s", title, authors);
  } else {
    snprintf(formatted, sizeof(formatted), "%s", title);
  }

  if (strcmp(formatted, lastMetadata) == 0) {
    return false;
  }

  strncpy(lastMetadata, formatted, sizeof(lastMetadata) - 1);
  lastMetadata[sizeof(lastMetadata) - 1] = '\0';

  copyString(formatted, buffer, bufferSize);
  return true;
}

void resetLastMetadata() {
  lastMetadata[0] = '\0';
}
