#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleLights.h"

extern volatile uint16_t buttonValue;
void cliParse(char *command) {
  DEBUG_VALUELN(DEBUG_MID, "received command=", command);
  if (strcmp(command, "mode") == 0) buttonValue++;
}

void cliRead() {
  static char input[MAX_CLI_LEN];
  static int i = 0;

  //  DEBUG_VALUELN(DEBUG_HIGH, "available=", Serial.available());
  while (Serial.available()) {
    char c = (char)Serial.read();
    //   DEBUG_VALUE(DEBUG_HIGH, " ", (byte)input[i]);

    if (c == '\n') {
      input[i+1] = 0;
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
