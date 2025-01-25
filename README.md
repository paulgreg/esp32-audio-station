# esp32-audio-station

An ESP32 bluetooth speaker and web radio, inspired by [KitchenRadio](https://github.com/jeroenlukas/KitchenRadio) and my previous bluetooth and web radio projects. That project was made thanks too [CelliesProjects/ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream) and [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) libraries ! 🙏 


## Features

 - 2 modes : Web Radio or Bluetooth receiver
 - mp3 and aac decoding (depends of VS1053 module)
 - OLED screen displaying radio, songs, volume
 - fetch radios from a json file (to easily update them)
 - IR remote (next / previous / volume + / volume - / mute / source)

## Status

I wasn’t able to switch correctly from bluetooth to wifi (not enought RAM when enabling wifi). So I store mode in preferences and reboot ESP32... 😐
When it reboots, it checks for mode and starts bluetooth or wifi.

## Hardware

 - ESP32-WROOM-32D/ESP32-WROOM-32U with Antenna
 - VS1003B VS1053 MP3 Module Development Board (make sure it decodes AAC for better stream quality)
 - 2.4" 128x64 OLED I2C SSD1309 Display Module
 - IR receiver
 - IR remote (from an old DVD player)


## Connections

  | ESP32  | VS1053 | OLED | IR |
  |--------|--------|------|----|
  |  GND   |   X    |  X   | G  |
  |  5V    |  5V    |      | R  |
  |  3.3V  |  3.3V  |      |    |
  |  IO18  |  SCK   |      |    |
  |  IO19  |  MISO  |      |    |
  |  IO23  |  MOSI  |      |    |
  |  EN    |  XRST  |      |    |
  |  IO5   |  CS    |      |    |
  |  IO16  |  DCS   |      |    |
  |  IO4   |  DREQ  |      |    |
  |  IO21  |        | SDA  |    |
  |  IO22  |        | SCL  |    |
  |  IO27  |        |      | Y  |


*Don’t use GND next to 5V ! Uploading will crash*

From [that example](https://github.com/baldram/ESP_VS1053_Library/blob/master/examples/WebRadioDemo/WebRadioDemo.ino).


## Configuration

Copy `parameters.h.dist` to `parameters.h` and change it to your settings.

⚠️ Web Radios are fetch from a JSON file. You'll need to create a JSON file containing web radios name and URL (see `WebRadios.h`) and host it somewhere on the internet.

Note : some VS1053 board aren't able to decode aac streams. You can usually find mp3 alternative (or change board).


## Librairies

- [paulgreg/ESP32_VS1053_Stream_raw](https://github.com/paulgreg/ESP32_VS1053_Stream_raw), forked from [CelliesProjects/ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream) to expose `playChunk`
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP.git)
- IRRemote

 `cd Arduino/libraries && git clone -b expose-play-chunk https//github.com/paulgreg/ESP32_VS1053_Stream_raw.git)`


## Flash

Flash using « ESP32 DEV Module » (or ESP32-WROOM-DA for my model)

⚠️ You’ll need to change layout to have enougth flash size : NO OTA (2 MB APP/2 MB SPIFFS)


## References

- [VS1053 Datasheet](https://www.sparkfun.com/datasheets/Components/SMD/vs1053.pdf)
- [ESP32_VS1053_Stream](https://github.com/CelliesProjects/ESP32_VS1053_Stream/)
- [ESP32-A2DP](https://github.com/pschatzmann/ESP32-A2DP) - [documentation](https://pschatzmann.github.io/ESP32-A2DP/html/class_bluetooth_a2_d_p_sink.html)
- https://www.instructables.com/HiFi-Online-Radio-Internet-Streaming-With-ESP32-an/
- to generate font : https://oleddisplay.squix.ch/
- https://www.makerguides.com/ir-receiver-remote-arduino-tutorial/
- [Inspiring issue to connect ESP32-A2DP to VS1053](https://github.com/pschatzmann/ESP32-A2DP/issues/31)
