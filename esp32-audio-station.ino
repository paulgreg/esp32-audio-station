#include "parameters.h"
#include "stringUtils.h"

#include "IRremote.hpp"

#include <Preferences.h>

#include "display.h"

#include <VS1053.h>

// Player with web radio stream handling
#include <ESP32_VS1053_Stream.h>

// bluetooth
#include "BluetoothA2DPSink.h"
#include <cbuf.h>
#include "bluetoothsink.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "WebRadios.h"
#include "network.h"
#include "fipmetadata.h"

#define STARTUP_DELAY_MS 3000
#define BT_START_DELAY_MS 100
#define MODE_SWITCH_DELAY_MS 250
#define ERROR_DISPLAY_DELAY_MS 10000
#define EOF_RESTART_DELAY_MS 1000
#define STREAM_LOOP_DELAY_MS 5

Preferences preferences;
ESP32_VS1053_Stream stream;

BluetoothA2DPSink a2dp_sink;

WebRadios webRadios;

unsigned long lastAction = millis();
unsigned long lastIRTime = 0;

unsigned short radioIdx = 0;
bool hasRadioIdxChanged = false;
bool radioIdxSaved = true;
unsigned int volume = VOLUME_MAX;
bool volumeSaved = true;
bool mute = false;
bool eof = false;
bool paused = false;
bool pendingRestart = false;

bool bluetoothMode = true;

unsigned long lastMetadataFetch = 0;

bool fetchWebRadiosData() {
  for (int retry = 0; retry < MAX_RETRY; retry++) {
    delay(RETRIES_DELAY);
    if (getWebRadiosJSON(&webRadios)) {
      return true;
    }
    Serial.printf("fetchWebRadiosData: retry %d/%d failed\n", retry + 1, MAX_RETRY);
  }
  return false;
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

  delay(STARTUP_DELAY_MS); // Wait for VS1053 and PAM8403 to power up
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);  /* start SPI before starting decoder */

  if (!stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !stream.isChipConnected()) {
    Serial.println("Decoder not running");
    while (1) delay(1000);
  }
  stream.setVolume(volume);
  stream.setStationCB(audio_showstation);
  stream.setInfoCB(audio_showstreamtitle);
  stream.setEofCB(audio_eof_stream);

  if (bluetoothMode) {
    copyString(BLUETOOTH_NAME, titleLabel, sizeof(titleLabel));
    a2dp_sink.set_stream_reader(read_data_stream, false);
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.start(BLUETOOTH_NAME);
    delay(BT_START_DELAY_MS);
    circBuffer.write((char *)bt_wav_header, 44);
    delay(BT_START_DELAY_MS);
  } else {
    displayText("Radio > Wifi");
    if (!connectToWifi()) {
      displayError("Wifi error");
      delay(ERROR_DISPLAY_DELAY_MS);
      toggleSource();
    } else {
      displayText("Radio > list");
      if (fetchWebRadiosData()) {
        radioIdx = radioIdx < webRadios.max ? radioIdx : 0;
        startRadio();
      } else {
        displayError("Radio : error");
        delay(ERROR_DISPLAY_DELAY_MS);
        toggleSource();
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
    if (pendingRestart) {
      pendingRestart = false;
      delay(EOF_RESTART_DELAY_MS);
      restartRadio();
    } else {
      if (stream.isRunning()) {
        stream.loop();
        pollRadioFranceMetadata();
        delay(STREAM_LOOP_DELAY_MS);
      }
      changeRadio();
    }
  }
  savePreferences();
  handleIRCommands();
}

void handleIRCommands() {
  if (IrReceiver.decode()) {
    bool isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
    if (!isRepeat && millis() - lastIRTime >= IR_DELAY) {
      lastIRTime = millis();
      lastAction = millis();
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
    }
    IrReceiver.resume();
  }
}

void startRadio() {
  Serial.printf("StartRadio %s - %s\n", webRadios.url[radioIdx], webRadios.name[radioIdx]);
  copyString(webRadios.name[radioIdx], titleLabel, sizeof(titleLabel));
  copyString("", songLabel, sizeof(songLabel));
  resetLastMetadata();
  lastMetadataFetch = 0;
  eof = false;
  refreshDisplay();
  stream.connectToHost(webRadios.url[radioIdx]);
}

void restartRadio() {
  Serial.printf("RestartRadio %s - %s\n", webRadios.url[radioIdx], webRadios.name[radioIdx]);
  resetLastMetadata();
  lastMetadataFetch = 0;
  eof = false;
  refreshDisplay();
  stream.connectToHost(webRadios.url[radioIdx]);
}

void changeRadioIndex(bool next) {
  if (stream.isRunning()) stream.stopSong();
  if (next) radioIdx = radioIdx < webRadios.max - 1 ? radioIdx + 1 : 0;
  else radioIdx = radioIdx > 0 ? radioIdx - 1 : webRadios.max - 1;
  copyString(webRadios.name[radioIdx], titleLabel, sizeof(titleLabel));
  copyString("", songLabel, sizeof(songLabel));
  resetLastMetadata();
  lastMetadataFetch = 0;
  refreshDisplay();
  hasRadioIdxChanged = true;
}

void toggleMute() {
  mute = !mute;
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
  preferences.putInt("volume", volume);
  preferences.putBool("bluetoothMode", !bluetoothMode);
  delay(MODE_SWITCH_DELAY_MS);
  ESP.restart();
}

void changeVolume(bool increase) {
  if (!mute) {
    volume = stream.getVolume();
    if (increase && volume <= (VOLUME_MAX - VOLUME_STEP)) volume += VOLUME_STEP;
    else if (!increase && volume >= VOLUME_STEP) volume -= VOLUME_STEP;
    volumeSaved = false;
    stream.setVolume(volume);
    refreshDisplay();
  }
}

void changeRadio () {
  if (hasRadioIdxChanged && hasTimePassed(CHANGE_RADIO_DELAY)) {
    hasRadioIdxChanged = false;
    radioIdxSaved = false;
    startRadio();
  }
}

void pollRadioFranceMetadata() {
  int metadataId = webRadios.radioFranceMetadataId[radioIdx];
  if (metadataId <= 0) return;
  if (millis() - lastMetadataFetch < METADATA_POLL_INTERVAL_MS) return;
  lastMetadataFetch = millis();
  char buffer[LABEL_BUFFER_SIZE];
  if (fetchRadioFranceMetadata(metadataId, buffer, sizeof(buffer))) {
    copyString(buffer, songLabel, sizeof(songLabel));
    refreshDisplay();
  }
}

void audio_showstation(const char* station) {
  Serial.printf("showstation: %s\n", station);
  char* aac = strstr(station, ".aac");
  char* mp3 = strstr(station, ".mp3");
  if (aac == NULL && mp3 == NULL) {
    copyString(station, titleLabel, sizeof(titleLabel));
    refreshDisplay();
  }
}

void audio_showstreamtitle(const char* song) {
  Serial.printf("streamtitle: %s\n", song);
  copyString(song, songLabel, sizeof(songLabel));
  refreshDisplay();
}

void audio_eof_stream(const char* error) {
  Serial.printf("End of stream: %s\n", error);
  eof = true;
  refreshDisplay();
  pendingRestart = true;
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
      preferences.putInt("radioIdx", radioIdx);
    }
    if (!volumeSaved) {
      Serial.println("should save volume");
      volumeSaved = true;
      preferences.putInt("volume", volume);
    }
  }
}
