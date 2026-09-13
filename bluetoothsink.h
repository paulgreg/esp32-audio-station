#define CIRC_BUFFER_SIZE (1024 * 24)
#define BT_AUDIO_BUFFSIZE 32

bool f_bluetoothsink_metadata_received = false;

cbuf circBuffer(CIRC_BUFFER_SIZE);

uint8_t mp3buff[BT_AUDIO_BUFFSIZE];

unsigned char bt_wav_header[44] = {
    0x52, 0x49, 0x46, 0x46, // RIFF
    0xFF, 0xFF, 0xFF, 0xFF, // size
    0x57, 0x41, 0x56, 0x45, // WAVE
    0x66, 0x6d, 0x74, 0x20, // fmt
    0x10, 0x00, 0x00, 0x00, // subchunk1size
    0x01, 0x00,             // audio format - pcm
    0x02, 0x00,             // numof channels
    0x44, 0xac, 0x00, 0x00, //, samplerate 44k1: 0x44, 0xac, 0x00, 0x00       48k: 48000: 0x80, 0xbb, 0x00, 0x00,
    0x10, 0xb1, 0x02, 0x00, //byterate
    0x04, 0x00,             // blockalign
    0x10, 0x00,             // bits per sample - 16
    0x64, 0x61, 0x74, 0x61, // subchunk3id -"data"
    0xFF, 0xFF, 0xFF, 0xFF  // subchunk3size (endless)
};

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2) {
    Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
    if (data1 == 0x1) { // Title
        strncpy(songLabel, (char *)data2, sizeof(songLabel) - 1);
        songLabel[sizeof(songLabel) - 1] = '\0';
    } else if (data1 == 0x2) {
        strncpy(titleLabel, (char *)data2, sizeof(titleLabel) - 1);
        titleLabel[sizeof(titleLabel) - 1] = '\0';
        f_bluetoothsink_metadata_received = true;
    }
}

void handle_stream(ESP32_VS1053_Stream *stream) {
  if (circBuffer.available()) {
      int bytesRead = circBuffer.read((char *)mp3buff, BT_AUDIO_BUFFSIZE);

      if (bytesRead != BT_AUDIO_BUFFSIZE) Serial.printf("Only read %d bytes from circular buffer\n", bytesRead);

      stream->playChunk(mp3buff, bytesRead, false);
  }
}

void read_data_stream(const uint8_t *data, uint32_t length) {
  if (circBuffer.room() > length) {
    if (length > 0) {
      circBuffer.write((char *)data, length);
    }
  } else {
    Serial.println("\nNothing to read from the stream");
  }
}
