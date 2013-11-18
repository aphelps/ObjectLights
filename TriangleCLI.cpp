#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleLights.h"

extern volatile uint16_t buttonValue;
extern pattern_args_t patternConfig;
void cliParse(char *command) {
  DEBUG_VALUELN(DEBUG_MID, "received command=", command);

  char *value_ptr = command;
  while (*value_ptr != 0) {
    if (*value_ptr == '=') {
      value_ptr++;
      break;
    }
    value_ptr++;
  }

  if (strncmp(command, "mode", 4) == 0) {
    int mode = atoi(value_ptr);
    buttonValue = mode;
  } else if (strncmp(command, "bgcolor", 6) == 0) {
    uint32_t color = strtol(value_ptr, NULL, 16);
    patternConfig.bgColor = color;
    DEBUG_VALUELN(DEBUG_MID, "Set bgcolor to ", color);
  } else if (strncmp(command, "fgcolor", 6) == 0) {
    uint32_t color = strtol(value_ptr, NULL, 16);
    patternConfig.fgColor = color;
    DEBUG_VALUELN(DEBUG_MID, "Set fgcolor to ", color);
  }
}

void cliRead() {
  static char input[MAX_CLI_LEN];
  static int i = 0;

  //  DEBUG_VALUELN(DEBUG_HIGH, "available=", Serial.available());
  while (Serial.available()) {
    char c = (char)Serial.read();
    //   DEBUG_VALUE(DEBUG_HIGH, " ", (byte)input[i]);

    if (c == '\n') {
      input[i] = 0;
      cliParse(input);
      i = 0;
    } else {
      input[i] = c;
      i++;
    }

    if (i >= MAX_CLI_LEN - 2) {
      // OVERFLOW
      i = 0;
    }
  }
}
