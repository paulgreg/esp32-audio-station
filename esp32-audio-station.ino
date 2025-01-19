#include "parameters.h"
#include "stringUtils.h"

#include "IRremote.hpp"

#include <Preferences.h>

#include "display.h"

#include <VS1053.h>

// Player with web radio stream handling
#include <ESP32_VS1053_Stream_raw.h>

// bluetooth
#include "BluetoothA2DPSink.h"
#include <cbuf.h>
#include "bluetoothsink.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "WebRadios.h"
#include "network.h"

Preferences preferences;
ESP32_VS1053_Stream_raw stream;

BluetoothA2DPSink a2dp_sink;

WebRadios webRadios;

unsigned long lastAction = millis();

unsigned short radioIdx = 0;
bool hasRadioIdxChanged = false;
bool radioIdxSaved = true;
unsigned int volume = VOLUME_MAX;
bool volumeSaved = true;
bool mute = false;
bool eof = false;
bool paused = false;

bool initialized = false;
bool bluetoothMode = true;

boolean fetchWebRadiosData() {
  boolean success = false;
  while(!success) {
    delay(RETRIES_DELAY);
    success = getWebRadiosJSON(&webRadios);
  }
  return success;
}

void setup() {
  titleLabel[0] = songLabel[0] ='\0';
  circBuffer.flush();

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);  

  preferences.begin("webradio", false);
  radioIdx = preferences.getInt("radioIdx", radioIdx);
  volume = preferences.getInt("volume", volume);
  bluetoothMode = preferences.getBool("bluetoothMode", bluetoothMode);

  Serial.begin(115200);
  Serial.println(bluetoothMode ? BLUETOOTH_NAME : "Web Radio");

  setupScreen();

  displayText(bluetoothMode ? BLUETOOTH_NAME : "Web Radio");

  delay(3000); // Wait for VS1053 and PAM8403 to power up
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);  /* start SPI before starting decoder */

  if (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !stream.isChipConnected()) { 
    Serial.println("Decoder not running");
    while (1) delay(1000);
  }

  if (bluetoothMode) {
    a2dp_sink.set_stream_reader(read_data_stream, false);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.start(BLUETOOTH_NAME);
    delay(100);
    circBuffer.write((char *)bt_wav_header, 44);
    delay(100);
    paused = false;
    initialized = true;
  } else {
    displayText("Radio > Wifi");
    if (!connectToWifi()) {
      displayError("Wifi error");
      delay(1000);
    } else {
      displayText("Radio > list");
      if (fetchWebRadiosData()) {
        radioIdx = radioIdx < webRadios.max ? radioIdx : 0;
        stream.setVolume(volume);
        startRadio();
        initialized = true;
      } else {
        displayError("Radio : error");
        delay(1000);
      }
    }
  }
}

void loop() {
  if (bluetoothMode) {
    handle_stream(&stream);
    if (f_bluetoothsink_metadata_received) {
      refreshDisplay();
      f_bluetoothsink_metadata_received = false;
    }
  } else {
    if (stream.isRunning()) {
      stream.loop();
      delay(5);
    }
    changeRadio();
  }
  savePreferences();
  handleIRCommands();
}

void handleIRCommands() {
 if (hasTimePassed(IR_DELAY) && IrReceiver.decode()) {
    uint16_t command = IrReceiver.decodedIRData.command;
    Serial.printf("IRcommand: %02x\n", command);
    switch (command) {
      case IR_PAUSE:
        if (bluetoothMode) {
          if (paused) {
            a2dp_sink.play();
          } else {
            a2dp_sink.pause();
          }
          paused = !paused;
        }
        break;
      case IR_NEXT:
        if (bluetoothMode) {
          a2dp_sink.next();
        } else {
          changeRadioIndex(true);
        }
        break;
      case IR_PREVIOUS:
        if (bluetoothMode) {
          a2dp_sink.previous();
        } else {
          changeRadioIndex(false);
        }
        break;
      case IR_VOL_UP: 
        changeVolume(true);
        break;
      case IR_VOL_DOWN:
        changeVolume(false);
        break;
      case IR_VOL_MUTE:
        toggleMute();
        break;
      case IR_VOL_SOURCE:
        toggleSource();
        break;
      default:	
        Serial.println("IRCommand: unknown");
    }
		IrReceiver.resume();
  }
}

void startRadio() {
  Serial.printf("StartRadio %s - %s\n", webRadios.url[radioIdx], webRadios.name[radioIdx]);
  copyString(webRadios.name[radioIdx], titleLabel);
  copyString("", songLabel);
  eof = false;
  refreshDisplay();
  stream.connecttohost(webRadios.url[radioIdx]);
  Serial.printf("codec: %s - bitrate: %lu kbps\n", stream.currentCodec(), stream.bitrate());
}

void restartRadio() {
  Serial.printf("RestartRadio %s - %s\n", webRadios.url[radioIdx], webRadios.name[radioIdx]);
  eof = false;
  stream.connecttohost(webRadios.url[radioIdx]);
}

void changeRadioIndex(bool next) {
  if (stream.isRunning()) stream.stopSong();
  if (next) radioIdx = radioIdx < webRadios.max - 1 ? radioIdx + 1 : 0;
  else radioIdx = radioIdx > 0 ? radioIdx - 1 : webRadios.max - 1;
  copyString(webRadios.name[radioIdx], titleLabel);
  copyString("", songLabel);
  refreshDisplay();
  hasRadioIdxChanged = true;
  lastAction = millis();
}

void toggleMute() {
  mute = stream.getVolume() == volume;
  unsigned int v = mute ? 0 : volume;
  stream.setVolume(v);
  refreshDisplay();
}

void toggleSource() {
  if (bluetoothMode) {
    a2dp_sink.stop();
    a2dp_sink.disconnect();
    a2dp_sink.end();
  } else {
    stream.stopSong();
  }
  preferences.putBool("bluetoothMode", !bluetoothMode);
  delay(250);
  ESP.restart();
}

void changeVolume(bool increase) {
  if (!mute) {
    volume = stream.getVolume();
    if (increase && volume <= (VOLUME_MAX - VOLUME_STEP)) volume += VOLUME_STEP;
    else if (!increase && volume >= VOLUME_STEP) volume -= VOLUME_STEP;
    volumeSaved = false;
    lastAction = millis();
    stream.setVolume(volume);
    refreshDisplay();
  }
}

void changeRadio () {
  if (hasRadioIdxChanged && hasTimePassed(CHANGE_RADIO_DELAY)) {
    hasRadioIdxChanged = false;
    radioIdxSaved = false;
    lastAction = millis();
    startRadio();
  }
}

void audio_showstation(const char* station) {
  Serial.printf("showstation: %s\n", station);
  char* aac = strstr(station, ".aac");
  char* mp3 = strstr(station, ".mp3");
  if (aac == NULL && mp3 == NULL) {
    copyString(station, titleLabel);
    refreshDisplay();
  }
}

void audio_showstreamtitle(const char* song) {
  Serial.printf("streamtitle: %s\n", song);
  copyString(song, songLabel);
  refreshDisplay();
}

void audio_eof_stream(const char* error) {
  Serial.printf("End of stream: %s\n", error);
  eof = true;
  refreshDisplay();
  delay(1000);
  restartRadio();
}

void refreshDisplay() {
  displayData(titleLabel, songLabel, volume, mute, eof);
}

bool hasTimePassed (unsigned long time) {
  return (millis() - lastAction) > time;
}

void savePreferences() {
  if (hasTimePassed(SAVE_DELAY)) {
    if (stream.isRunning() && !radioIdxSaved) {
      Serial.println("should save radio index");
      radioIdxSaved = true;
      if (!preferences.isKey("radioIdx") || preferences.getInt("radioIdx", radioIdx) != radioIdx) {
        Serial.printf("!!! Save radio index %i\n", radioIdx);
        preferences.putInt("radioIdx", radioIdx);
      }
    }
     if (!volumeSaved) {
      Serial.println("should save volume");
      volumeSaved = true;
      if (!preferences.isKey("volume") || preferences.getInt("volume", volume) != volume) {
        Serial.printf("!!! Save volume %i\n", volume);
        preferences.putInt("volume", volume);
      }
    }
  }
}