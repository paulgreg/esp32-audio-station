# esp32-audio-station

An ESP32 bluetooth speaker and web radio, inspired by [KitchenRadio](https://github.com/jeroenlukas/KitchenRadio) and my previous bluetooth and web radio projects. That project was made thanks to [CelliesProjects/ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream) and [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) libraries! 🙏

## Features

- 2 modes: Web Radio or Bluetooth receiver
- mp3 and aac decoding (depends on VS1053 module)
- OLED screen displaying radio, songs, volume
- fetch radios from a JSON file (to easily update them)
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
- `MAX_RETRY`: number of retries before giving up on fetching radios (default: 10)
- `DEBUG`: set to `true` to enable verbose serial output (default: `false`)

⚠️ Web Radios are fetched from a JSON file. You'll need to create a JSON file containing web radios name and URL and host it somewhere on the internet. `radios.json` example:

    [
      {"name":"Fip","url":"http://icecast.radiofrance.fr/fip-hifi.aac"},
      {"name":"Fip Jazz","url":"http://icecast.radiofrance.fr/fipjazz-hifi.aac"},
      {"name":"France Musique","url":"http://icecast.radiofrance.fr/francemusique-hifi.aac"},
      {"name":"Bossa Nova","url":"http://54.38.43.201:8009/stream-128kmp3-BossaNovaBrazil"},
      {"name":"The Wave","url":"http://75.102.53.58/1066"},
      {"name":"The Lounge","url":"http://64.95.243.43:8020/stream"},
      {"name":"Funky Radio","url":"http://176.31.111.65:4744"},
      {"name":"80s","url":"http://ice1.somafm.com/u80s-128-mp3" },
      {"name":"Classic","url":"http://media-ice.musicradio.com/ClassicFMMP3"},
      {"name":"Night Ride","url":"http://stream.nightride.fm/nightride.mp3"}
    ]

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

⚠️ You'll need to change partition layout to have enough flash size: NO OTA (2 MB APP/2 MB SPIFFS)


## References

- [VS1053 Datasheet](https://www.sparkfun.com/datasheets/Components/SMD/vs1053.pdf)
- [ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream/)
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) - [documentation](https://pschatzmann.github.io/ESP32-A2DP/html/class_bluetooth_a2_d_p_sink.html)
- https://www.instructables.com/HiFi-Online-Radio-Internet-Streaming-With-ESP32-an/
- to generate font : https://oleddisplay.squix.ch/
- https://www.makerguides.com/ir-receiver-remote-arduino-tutorial/
- [Inspiring issue to connect ESP32-A2DP to VS1053](https://github.com/pschatzmann/ESP32-A2DP/issues/31)
