void copyString(const char* str, char* buffer) {
  unsigned int len = strlen(str);
  strncpy(buffer, str, len);
  buffer[len] = '\0';
}

void capitalizeWords(char *str) {
    // Check if the string is not empty
    if (str != NULL) {
        int i=0;

        str[i] = toupper(str[0]);
        i++;//increment after every look
        while(str[i] != '\0') {
            if(isspace(str[i])) {
                str[i] = str[i];
                str[i+1] = toupper(str[i+1]);
                i+=2;//look twice, increment twice
            } else {       
                i++;//increment after every look
            }
        }
    }
}

void convertUtf8ToIso88591Hex(char* input) {
    char buffer[512];  // Temporary buffer for the converted string
    int bufferIndex = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        unsigned char c = (unsigned char)input[i];

        if (c < 0x80) {
            // ASCII character, copy directly
            buffer[bufferIndex++] = c;
        } else if (c >= 0xC2 && c <= 0xDF) {
            // UTF-8 two-byte sequence for ISO-8859-1 characters
            unsigned char next = (unsigned char)input[++i];
            buffer[bufferIndex++] = ((c & 0x1F) << 6) | (next & 0x3F);
        } else {
            // Unsupported character, replace with '?'
            buffer[bufferIndex++] = '?';
        }
    }

    buffer[bufferIndex] = '\0';  // Null-terminate the new string

    // Copy the modified string back to the input
    strcpy(input, buffer);
}

void formatString (const char* str, char* buffer, unsigned int limit) {
  unsigned int len = strlen(str);
  if (len == 0) {
    buffer[0] = '\0';
  } else {
    unsigned int c = len > limit ? limit : len;
    strncpy(buffer, str, c);
    convertUtf8ToIso88591Hex(buffer);
    capitalizeWords(buffer);
    buffer[c] = '\0';
  }
}