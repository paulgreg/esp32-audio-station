void copyString(const char* str, char* buffer, unsigned int bufferSize) {
  unsigned int len = strlen(str);
  if (len >= bufferSize) len = bufferSize - 1;
  strncpy(buffer, str, len);
  buffer[len] = '\0';
}

void capitalizeWords(char *str) {
    if (str != NULL && str[0] != '\0') {
        int i = 0;
        str[i] = toupper(str[0]);
        i++;
        while(str[i] != '\0') {
            if(isspace(str[i])) {
                if (str[i+1] != '\0') {
                    str[i+1] = toupper(str[i+1]);
                }
                i += 2;
            } else {
                i++;
            }
        }
    }
}

void convertUtf8ToIso88591Hex(char* input) {
    char buffer[512];
    int bufferIndex = 0;

    for (int i = 0; input[i] != '\0' && bufferIndex < (int)sizeof(buffer) - 1; i++) {
        unsigned char c = (unsigned char)input[i];

        if (c < 0x80) {
            buffer[bufferIndex++] = c;
        } else if (c >= 0xC2 && c <= 0xDF) {
            unsigned char next = (unsigned char)input[++i];
            buffer[bufferIndex++] = ((c & 0x1F) << 6) | (next & 0x3F);
        } else {
            buffer[bufferIndex++] = '?';
        }
    }

    buffer[bufferIndex] = '\0';
    strcpy(input, buffer);
}

void formatString (const char* str, char* buffer, unsigned int limit) {
  unsigned int len = strlen(str);
  if (len == 0) {
    buffer[0] = '\0';
  } else {
    unsigned int c = len > limit ? limit : len;
    strncpy(buffer, str, c);
    buffer[c] = '\0';
    convertUtf8ToIso88591Hex(buffer);
    capitalizeWords(buffer);
  }
}
