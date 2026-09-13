#define RADIO_NAME_SIZE 64
#define RADIO_URL_SIZE 127

struct WebRadios {
  unsigned int max;
  char name[MAX_RADIOS][RADIO_NAME_SIZE];
  char url[MAX_RADIOS][RADIO_URL_SIZE];
};

void fillWebRadiosFromJson(JSONVar json, WebRadios* webradios) {
  int size = json.length();
  int count = size < MAX_RADIOS ? size : MAX_RADIOS;
  for (int i = 0; i < count; i++) {
    snprintf(webradios->name[i], sizeof(webradios->name[i]), "%s", (const char*) json[i]["name"]);
    snprintf(webradios->url[i], sizeof(webradios->url[i]), "%s", (const char*) json[i]["url"]);
  }
  webradios->max = count;
}
