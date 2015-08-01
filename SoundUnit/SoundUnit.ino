/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

/*
 *  The code for sound detection and equilization is largely taken from the
 *  Adafruit PICCOLO project:
 *     https://github.com/adafruit/piccolo
 */

// IMPORTANT: FFT_N should be #defined as 128 in ffft.h.  This is different
// than Spectro, which requires FFT_N be 64 in that file when compiling.


#define DEBUG_LEVEL 4
#include <Debug.h>

#include <avr/pgmspace.h>
#include <ffft.h>
#include <math.h>

#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>
#include "SPI.h"
#include "FastLED.h"

#include "SerialCLI.h"

#include "PixelUtil.h"

#include "Socket.h"
#include "RS485Utils.h"

#include "SoundUnit.h"

// Note: These are only needed for ArduinoIDE compilation.  With this environment
// make sure to set DISABLE_MPR121 in the HMTLTypes.h, otherwise the static
// memory allocation (blame Wire.h) will exceed some limit and this won't work
#include "EEPROM.h"
#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "Wire.h"
#include "MPR121.h"
// End note

#include "HMTLTypes.h"
#include "HMTLMessaging.h"
#include "HMTLProtocol.h"

#define NUM_OUTPUTS 3
config_rgb_t rgb_output;
RS485Socket rs485;
PixelUtil pixels;

#define BAUD 115200

SerialCLI serialcli(32, cliHandler);

void setup() {
  Serial.begin(BAUD);
  DEBUG2_PRINTLN("*** SoundUnit starting ***");

  config_hdr_t config;
  config_max_t readoutputs[NUM_OUTPUTS];
  int32_t outputs_found = hmtl_setup(&config, readoutputs,
                                     NULL, NULL, NUM_OUTPUTS,
                                     &rs485, NULL, &pixels, NULL,
                                     &rgb_output, NULL,
                                     NULL);

  pixels.setAllRGB(128, 0, 128);
  pixels.update();

  messaging_init();


  // Setup the sound sensing
  sound_initialize();

  DEBUG2_PRINTLN("*** SoundUnit initialized ***");
  //  Serial.println(F(HMTL_READY));
}

boolean sentResponse = false;

void loop() {
  static unsigned long lastData = 0;

  /* Check for and handle messages over the RS485 interface */
  if (messaging_handle()) {
    sentResponse = true;
  }

  if (check_sound()) {
    print_data();
    set_leds();

    sentResponse = false;

    digitalWrite(rgb_output.pins[0], 255);
    lastData = millis();
  }

  if (millis() - lastData > 5) {
    digitalWrite(rgb_output.pins[0], 0);
  }

  serialcli.checkSerial();
}

uint16_t print_timer = 0;

/*
 * Print the sound levels
 */
void print_data() {
  uint16_t total;

  print_timer++;

  if ((print_timer >= output_period) || sentResponse) {
    print_timer = 0;

    if (verbosity >= 2) {
      total = 0;
      DEBUG4_PRINT("Post eq:");
      for(uint16_t x = 0; x < FFT_N / 2; x++) {
        DEBUG4_VALUE(" ", spectrum[x]);
        total += spectrum[x];
      }
      DEBUG4_VALUE(" +", total);
    }

    if (verbosity >= 1) {
      total = 0;
      DEBUG4_PRINT(" col:");
      for (byte c = 0; c < NUM_COLUMNS; c++) {
        DEBUG4_VALUE(" ", col[c][colCount]);
        total += col[c][colCount];
      }
      DEBUG4_VALUE(" +", total);

      total = 0;
      DEBUG4_PRINT(" lvl:");
      for (byte c = 0; c < NUM_COLUMNS; c++) {
        DEBUG4_VALUE(" ", colLeveled[c]);
        total += colLeveled[c];
      }
      DEBUG4_VALUE(" +", total);

      DEBUG4_VALUE(" l:", light_level);
      DEBUG4_VALUE(" k:", knob_level);
    }

    if (verbosity >= 3) {
      DEBUG4_PRINT(" raw:");
      for (uint16_t x = 0; x < FFT_N; x++) {
        DEBUG4_VALUE(" ", capture[x]);
      }
    }

    if (sentResponse) {
      DEBUG4_PRINT(" Sent");
    }
  }

  DEBUG_PRINT_END();
}

/*
 * Set the LEDs based on sound levels
 */
void set_leds() {
  uint32_t total = 0;

  uint16_t led = 1;
  
  for (byte c = 0; c < NUM_COLUMNS; c++) {
    total += colLeveled[c];
    if (c % 2 == 1) {
      byte heat = (total > 15 ? 255 : total * total);
      pixels.setPixelRGB(led, pixel_heat(heat));
      led++;
      total = 0;
    }
  }

  pixels.update();
}
