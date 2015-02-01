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

/* TODO: Factoring out MPR121 from HMTLTypes.h will probably allow these to be
   included.
#include "HMTLTypes.h"
#include "HMTLMessaging.h"
#include "HMTLProtocol.h"
*/

#include "SerialCLI.h"

#include "PixelUtil.h"

#include "Socket.h"
#include "RS485Utils.h"

#include "SoundUnit.h"

#define LED_RED   10
#define LED_GREEN 11
#define LED_BLUE  13

PixelUtil pixels; //(4, 12, 8, RGB);
RS485Socket rs485(4, 7, 5, false);

/*
 * For sending sound data back over RS485.  Set the buffer size large enough
 * to allow for all spectrum data
 */
#define SEND_BUFFER_SIZE RS485_BUFFER_TOTAL(sizeof (uint16_t) * NUM_COLUMNS + 1)
byte rs485_buffer[SEND_BUFFER_SIZE];
byte *send_buffer; // Pointer to use for start of send data
#define MY_ADDR 0x01

#define BAUD 115200

SerialCLI serialcli(32, cliHandler);

void setup() {
  Serial.begin(115200);
  DEBUG2_PRINTLN("*** SoundUnit starting ***");

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  pixels = PixelUtil(5, 12, 8, RGB); //pixels.init(5, 12, 8, RGB);
  pixels.setAllRGB(128, 0, 128);
  pixels.update();

  /* Setup the RS485 connection */
  rs485.setup();
  send_buffer = rs485.initBuffer(rs485_buffer);

  // Setup the sound sensing
  sound_initialize();

  DEBUG2_PRINTLN("*** SoundUnit initialized ***");
  //  Serial.println(F(HMTL_READY));
}

boolean sentResponse = false;

void loop() {
  static unsigned long lastData = 0;

  /* Check for and handle messages over the RS485 interface */
  if (handleMessages()) {
    sentResponse = true;
  }

  if (check_sound()) {
    output_data();
    set_leds();

    sentResponse = false;

    digitalWrite(LED_RED, 255);
    lastData = millis();
  }

  if (millis() - lastData > 5) {
    digitalWrite(LED_RED, 0);
  }

  serialcli.checkSerial();
}


/*
 * Listen for RS485 messages and respond with sound data
 */
boolean handleMessages() {
  uint16_t msglen;
  const byte *data = rs485.getMsg(MY_ADDR, &msglen);
  if (data != NULL) {
    uint16_t response_len = 0;

    DEBUG5_VALUE("Recv: command=", data[0]);
    DEBUG5_VALUE(" len=", msglen);
    DEBUG5_VALUE(" data=", (char)data[0]);

    if (RS485_ADDRESS_FROM_DATA(data) = MY_ADDR) {
      switch (data[0]) {
        /* TODO: Spectrum data is too big to justify the additional send buffer,
           send in multiple packets instead.
           case 'S': {
           // Return the entire spectrum array
           uint16_t *sendPtr = (uint16_t *)send_buffer;
           for (byte x = 0; x < FFT_N/2; x++) {
           *sendPtr = spectrum[x];
           sendPtr++;
           }
           response_len = ((uint16_t)sendPtr - (uint16_t)send_buffer);
           break;
           }
        */
        case 'C': {
          // Return the current raw column values
          uint16_t *sendPtr = (uint16_t *)send_buffer;
          for (byte c = 0; c < NUM_COLUMNS; c++) {
            *sendPtr = col[c][colCount];
            sendPtr++;
          }
          response_len = ((uint16_t)sendPtr - (uint16_t)send_buffer);
          break;
        }
        case 'L': {
          // Return the current leveled column values
          uint8_t *sendPtr = (uint8_t *)send_buffer;
          for (byte c = 0; c < NUM_COLUMNS; c++) {
            *sendPtr = colLeveled[c];
            sendPtr++;
          }
          response_len = ((uint16_t)sendPtr - (uint16_t)send_buffer);
          break;
        }
      }
    }

    if (response_len > 0) {
      DEBUG5_VALUE(" retlen=", response_len);
      uint16_t source_address = RS485_SOURCE_FROM_DATA(data);
      DEBUG5_VALUE(" retaddr=", source_address);
      rs485.sendMsgTo(source_address, send_buffer, response_len);
    }

    DEBUG_PRINT_END();
    return true;
  }

  return false;
}

uint16_t print_timer = 0;

/*
 * Print the sound levels
 */
void output_data() {
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
