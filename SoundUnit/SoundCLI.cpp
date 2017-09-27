/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL DEBUG_HIGH
#endif
#include "Debug.h"

#include "SoundUnit.h"
#include "SerialCLI.h"

byte verbosity = 2;
uint16_t output_period = 10;
byte output_mode = OUTPUT_MODE_TEXT;

void print_usage() {
Serial.print(F(" \n"
  "Usage:\n"
  "  v <num> - Set verbosity level\n"
  "  p <ms>  - Period in milliseconds\n"
  "  m - Print the current milliseconds\n"
  "  n - Disable serial output\n"
  "  t - Text serial output mode\n"
  "  b - Binary serial output mode\n"
  "  h - Print this help\n"
               ));
}             
void cliHandler(char **tokens, byte numtokens) {
  switch (tokens[0][0]) {
    case 'v': {
      if (numtokens < 2) return;
      verbosity = atoi(tokens[1]);
      DEBUG2_VALUELN("Set verbose:", verbosity);
      break;
    }

    case 'p': {
      if (numtokens < 2) return;
      output_period = atoi(tokens[1]);
      DEBUG2_VALUELN("Set output period:", output_period);
      break;
    }

    case 'm': {
      DEBUG2_VALUELN("millis:", millis());
      break;
    }

    case 'n': {
      DEBUG2_PRINTLN("Set output mode: none");
      output_mode = OUTPUT_MODE_NONE;
      break;
    }
    case 't': {
      DEBUG2_PRINTLN("Set output mode: text");
      output_mode = OUTPUT_MODE_TEXT;
      break;
    }
    case 'b': {
      DEBUG2_PRINTLN("Set output mode: binary");
      output_mode = OUTPUT_MODE_BINARY;
      break;
    }

    case '?':
    case 'h': {
      print_usage();
    }
  }
}
