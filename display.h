#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// #include "font-roboto.h"
// #define FONT_SMALL  &Roboto_Medium_12
// #define FONT_MEDIUM &Roboto_Medium_14
// #define FONT_BIG    &Roboto_Medium_16

#include "RobotoCondensedRegular_euro8pt8b.h"
#include "RobotoCondensedRegular_euro9pt8b.h"
#include "RobotoCondensedRegular_euro10pt8b.h"

#define FONT_SMALL  &RobotoCondensed_Regular8pt8b
#define FONT_MEDIUM &RobotoCondensed_Regular9pt8b
#define FONT_BIG    &RobotoCondensed_Regular10pt8b

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char titleLabel[255];
char songLabel[255];

void setupScreen() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setRotation(2);
  display.cp437(true);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void displayText(const char* s) {
  display.clearDisplay();
  display.setFont(FONT_BIG);
  display.setCursor(10, 20);
  display.print(s);
  display.display();
}

void displayError(const char* s) {
  Serial.println(s);
  display.clearDisplay();
  display.invertDisplay(true);
  display.setFont(FONT_BIG);
  display.setCursor(10, 40);
  display.print(s);
  display.display();
}

char titleBuffer[255];
char songBuffer[255];

void displayData(const char* title, const char* song, unsigned int volume, bool mute, bool eof) {
  Serial.printf("title « %s », song « %s », vol: %i, mute: %i, eof: %i\n", title, song, volume, mute, eof);

  formatString(title, titleBuffer, TITLE_LEN_LIMIT);
  formatString(song, songBuffer, SONG_LEN_LIMIT);

  display.clearDisplay();
  if (eof) { // End of stream
    display.setFont(FONT_MEDIUM);
    display.setCursor(0, 15);
    display.print(titleBuffer);
    display.setFont();
    display.setCursor(0, 35);
    display.print("Stream error");
  } else if (strlen(song) == 0) { // only title name
    display.setFont(FONT_BIG);
    display.setCursor(0, 25);
    display.print(titleBuffer);
  } else { // with title and song
    display.setFont();
    display.setFont(FONT_MEDIUM);
    display.setCursor(0, 12);
    display.print(titleBuffer);
    display.setFont(FONT_SMALL);
    display.setCursor(0, 32);
    display.print(songBuffer);
  }

  display.setFont(FONT_SMALL);
  display.setCursor(90, 62);
  if (mute) {
    display.printf("  - %%");
  } else {
    display.printf("%*i %%", 3, volume);
  }

  display.display();
}