#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "SoundUnit.h"
#include "SerialCLI.h"

byte verbosity = 2;
uint16_t output_period = 10;

void print_usage() {
Serial.print(F(" \n"
  "Usage:\n"
  "  v <num> - Set verbosity level\n"
  "  p <ms>  - Period in milliseconds\n"
  "  m - Print the current milliseconds\n"
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

    case 'h': {
      print_usage();
    }
  }
}
