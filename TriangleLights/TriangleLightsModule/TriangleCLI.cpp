/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2016
 *
 * Main sketch for running TriangleLights
 ******************************************************************************/

#include <Arduino.h>

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL DEBUG_HIGH
#endif
#include "Debug.h"

#include "SerialCLI.h"

#include "TriangleLights.h"

extern volatile uint16_t buttonValue;

void cliHandler(char **tokens, byte numtokens);

SerialCLI serialcli(
        MAX_CLI_LEN, // Max command length, this amount is allocated as a buffer
        cliHandler   // Function for handling tokenized commands
);

void print_usage() {
  DEBUG3_PRINTLN(
" \n"
"Usage:\n"
"  h - print this help\n"
"  m <mode> - Set the mode\n"
"  b <color> - bgcolor color\n"
"  f <color> - fgcolor color\n"
);
}
void cliHandler(char **tokens, byte numtokens) {
  switch (tokens[0][0]) {
    case 'h': {
      print_usage();
      break;
    }

    case 'm': {
      if (numtokens < 2) return;
      uint16_t mode = atoi(tokens[1]);
      buttonValue = mode;
      DEBUG3_VALUELN("Set mode:", mode);
      break;
    }

#if 0
    case 'b': {
      if (numtokens < 2) return;
      uint32_t color = strtol(tokens[1], NULL, 16);
      patternConfig.bgColor = color;
      DEBUG3_HEXVALLN("Set bgcolor to", color);
      break;
    }

    case 'f': {
      if (numtokens < 2) return;
      uint32_t color = strtol(tokens[1], NULL, 16);
      patternConfig.fgColor = color;
      DEBUG3_HEXVALLN("Set fgcolor to", color);
      break;
    }
#endif
  }
}