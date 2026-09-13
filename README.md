# esp32-audio-station

An ESP32 bluetooth speaker and web radio, inspired by [KitchenRadio](https://github.com/jeroenlukas/KitchenRadio) and my previous bluetooth and web radio projects. That project was made thanks to [CelliesProjects/ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream) and [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) libraries! 🙏

## Features

- 2 modes: Web Radio or Bluetooth receiver
- mp3 and aac decoding (depends on VS1053 module)
- OLED screen displaying radio, songs, volume
- fetch radios from a JSON file (to easily update them)
- song metadata for Radio France stations (FIP, France Inter, etc.) via the Radio France live metadata API, fetched asynchronously on a separate FreeRTOS task (Core 0) to avoid audio glitches
- IR remote (next / previous / volume + / volume - / mute / source)
- configurable debug output and retry limits

## Status

I wasn't able to switch correctly from bluetooth to wifi (not enough RAM when enabling wifi). So I store mode in preferences and reboot ESP32... 😐
When it reboots, it checks for mode and starts bluetooth or wifi.

## Hardware

- ESP32-WROOM-32D/ESP32-WROOM-32U with Antenna
- VS1003B VS1053 MP3 Module Development Board (make sure it decodes AAC for better stream quality)
- 2.4" 128x64 OLED I2C SSD1306 Display Module
- IR receiver
- IR remote (from an old DVD player)


## Connections

  | ESP32  | VS1053 | OLED | IR |
  |--------|--------|------|----|
  | GND    | X      | X    | G  |
  | 5V     | 5V     |      | R  |
  | 3.3V   | 3.3V   |      |    |
  | IO18   | SCK    |      |    |
  | IO19   | MISO   |      |    |
  | IO23   | MOSI   |      |    |
  | EN     | XRST   |      |    |
  | IO5    | CS     |      |    |
  | IO16   | DCS    |      |    |
  | IO4    | DREQ   |      |    |
  | IO21   |        | SDA  |    |
  | IO22   |        | SCL  |    |
  | IO27   |        |      | Y  |

*Don't use GND next to 5V! Uploading will crash*

From [that example](https://github.com/baldram/ESP_VS1053_Library/blob/master/examples/WebRadioDemo/WebRadioDemo.ino).


## Configuration

Copy `parameters.h.dist` to `parameters.h` and change it to your settings.

Key parameters:
- `WIFI_SSID` / `WIFI_PASSWORD`: WiFi credentials
- `BLUETOOTH_NAME`: Bluetooth device name
- `WEB_RADIOS_URL`: URL to your JSON file containing web radios
- `WEB_RADIO_USER` / `WEB_RADIO_PASSWD`: HTTP auth credentials (can be empty)
- `RADIO_FRANCE_LIVEMETA_URL`: Base URL for the Radio France live metadata API (default: `https://api.radiofrance.fr/livemeta/pull/`)
- `METADATA_POLL_INTERVAL_MS`: How often to poll metadata (default: 15000ms)
- `MAX_RETRY`: number of retries before giving up on fetching radios (default: 10)
- `DEBUG`: set to `true` to enable verbose serial output (default: `false`)

⚠️ Web Radios are fetched from a JSON file. You'll need to create a JSON file containing web radios name and URL and host it somewhere on the internet. See `radios` folder for exemple.

Radio France stations (FIP, France Inter, etc.) don't embed song metadata in their audio streams. To display song titles for these stations, add an optional `radioFranceMetadataId` field to the JSON entry. The ESP32 polls the [Radio France live metadata API](https://api.radiofrance.fr/livemeta/pull/) every ~15 seconds to fetch the current song title and artist. This fetch runs on a **dedicated FreeRTOS task pinned to Core 0**, so the HTTPS request never blocks the audio streaming loop on Core 1. Stations without this field are not polled. `radios.json` example:

    [
      {"name":"Fip","url":"http://icecast.radiofrance.fr/fip-hifi.aac","radioFranceMetadataId":7},
      {"name":"Fip Jazz","url":"http://icecast.radiofrance.fr/fipjazz-hifi.aac","radioFranceMetadataId":74},
      {"name":"Radio Nova","url":"https://radionova.ice.infomaniak.ch/radionova-256.aac"}
    ]

Known Radio France metadata IDs:

| Station | ID |
|---------|---|
| FIP | 7 |
| FIP Rock | 64 |
| FIP Groove | 65 |
| FIP Electro | 66 |
| FIP World | 69 |
| FIP Pop | 70 |
| FIP Reggae | 71 |
| FIP Jazz | 74 |
| FIP Metal | 77 |

Note: some VS1053 boards aren't able to decode aac streams. You can usually find mp3 alternatives (or change board).


## Libraries

- [CelliesProjects/ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream)
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP.git)
- [Arduino-IRremote](https://github.com/Arduino-IRremote/Arduino-IRremote)

```
cd Arduino/libraries && git clone https://github.com/CelliesProjects/ESP32_VS1053_Stream.git
cd Arduino/libraries && git clone https://github.com/pschatzmann/ESP32-A2DP.git
```

Other libraries (`Adafruit_SSD1306`, `Adafruit_GFX`, `Arduino_JSON`) can be installed via the Arduino Library Manager.


## Flash

Flash using "ESP32 DEV Module" (or ESP32-WROOM-DA for my model)

⚠️  You'll need to change partition layout to have enough flash size: NO OTA (2 MB APP/2 MB SPIFFS)


This is required because fetching Radio France song metadata uses HTTPS (`WiFiClientSecure`), which pulls in the TLS/mbedtls stack (~300-500 KB). The default partition scheme (1.2 MB APP + OTA) is not large enough. In the Arduino IDE, select **Tools > Partition Scheme > "NO OTA (2MB APP/2MB SPIFFS)"**. If using PlatformIO, add `board_build.partitions = no_ota.csv` to `platformio.ini`.


## References

- [VS1053 Datasheet](https://www.sparkfun.com/datasheets/Components/SMD/vs1053.pdf)
- [ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream/)
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) - [documentation](https://pschatzmann.github.io/ESP32-A2DP/html/class_bluetooth_a2_d_p_sink.html)
- https://www.instructables.com/HiFi-Online-Radio-Internet-Streaming-With-ESP32-an/
- to generate font : https://oleddisplay.squix.ch/
- https://www.makerguides.com/ir-receiver-remote-arduino-tutorial/
- [Inspiring issue to connect ESP32-A2DP to VS1053](https://github.com/pschatzmann/ESP32-A2DP/issues/31)
