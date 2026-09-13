#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "RobotoCondensedRegular_euro8pt8b.h"
#include "RobotoCondensedRegular_euro9pt8b.h"
#include "RobotoCondensedRegular_euro10pt8b.h"

#define FONT_SMALL  &RobotoCondensed_Regular8pt8b
#define FONT_MEDIUM &RobotoCondensed_Regular9pt8b
#define FONT_BIG    &RobotoCondensed_Regular10pt8b

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1

#define LABEL_BUFFER_SIZE 128

#define DISPLAY_CURSOR_TITLE_X 0
#define DISPLAY_CURSOR_TITLE_Y 25
#define DISPLAY_CURSOR_TITLE_EOF_X 0
#define DISPLAY_CURSOR_TITLE_EOF_Y 15
#define DISPLAY_CURSOR_SONG_EOF_X 0
#define DISPLAY_CURSOR_SONG_EOF_Y 35
#define DISPLAY_CURSOR_TITLE_BOTH_X 0
#define DISPLAY_CURSOR_TITLE_BOTH_Y 12
#define DISPLAY_CURSOR_SONG_BOTH_X 0
#define DISPLAY_CURSOR_SONG_BOTH_Y 32
#define DISPLAY_CURSOR_VOLUME_X 90
#define DISPLAY_CURSOR_VOLUME_Y 62
#define DISPLAY_CURSOR_TEXT_X 10
#define DISPLAY_CURSOR_TEXT_Y 20
#define DISPLAY_CURSOR_ERROR_Y 40

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char titleLabel[LABEL_BUFFER_SIZE];
char songLabel[LABEL_BUFFER_SIZE];

void setupScreen() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.setRotation(2);
  display.cp437(true);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void displayText(const char* s) {
  display.clearDisplay();
  display.invertDisplay(false);
  display.setFont(FONT_BIG);
  display.setCursor(DISPLAY_CURSOR_TEXT_X, DISPLAY_CURSOR_TEXT_Y);
  display.print(s);
  display.display();
}

void displayError(const char* s) {
  Serial.println(s);
  display.clearDisplay();
  display.invertDisplay(true);
  display.setFont(FONT_BIG);
  display.setCursor(DISPLAY_CURSOR_TEXT_X, DISPLAY_CURSOR_ERROR_Y);
  display.print(s);
  display.display();
}

char titleBuffer[LABEL_BUFFER_SIZE];
char songBuffer[LABEL_BUFFER_SIZE];

void displayData(const char* title, const char* song, unsigned int volume, bool mute, bool eof) {
  Serial.printf("title « %s », song « %s », vol: %i, mute: %i, eof: %i\n", title, song, volume, mute, eof);

  formatString(title, titleBuffer, TITLE_LEN_LIMIT);
  formatString(song, songBuffer, SONG_LEN_LIMIT);

  display.clearDisplay();
  display.invertDisplay(false);
  if (eof) {
    display.setFont(FONT_MEDIUM);
    display.setCursor(DISPLAY_CURSOR_TITLE_EOF_X, DISPLAY_CURSOR_TITLE_EOF_Y);
    display.print(titleBuffer);
    display.setFont();
    display.setCursor(DISPLAY_CURSOR_SONG_EOF_X, DISPLAY_CURSOR_SONG_EOF_Y);
    display.print("Stream error");
  } else if (strlen(song) == 0) {
    display.setFont(FONT_BIG);
    display.setCursor(DISPLAY_CURSOR_TITLE_X, DISPLAY_CURSOR_TITLE_Y);
    display.print(titleBuffer);
  } else {
    display.setFont(FONT_MEDIUM);
    display.setCursor(DISPLAY_CURSOR_TITLE_BOTH_X, DISPLAY_CURSOR_TITLE_BOTH_Y);
    display.print(titleBuffer);
    display.setFont(FONT_SMALL);
    display.setCursor(DISPLAY_CURSOR_SONG_BOTH_X, DISPLAY_CURSOR_SONG_BOTH_Y);
    display.print(songBuffer);
  }

  display.setFont(FONT_SMALL);
  display.setCursor(DISPLAY_CURSOR_VOLUME_X, DISPLAY_CURSOR_VOLUME_Y);
  if (mute) {
    display.printf("  - %%");
  } else {
    display.printf("%*i %%", 3, volume);
  }

  display.display();
}
