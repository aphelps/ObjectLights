

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include <NewPing.h>

#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "FastLED.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "Wire.h"
#include "MPR121.h"
#include "SerialCLI.h"

#include "Socket.h"
#include "RS485Utils.h"
#include "XBee.h"
#include "XBeeSocket.h"

#include "HMTLMessaging.h"

#include "TriangleLights.h"

/******/

config_hdr_t config;
output_hdr_t *outputs[HMTL_MAX_OUTPUTS];
config_hdr_t readconfig;
config_max_t readoutputs[HMTL_MAX_OUTPUTS];

config_rgb_t rgb_output;
config_pixels_t pixel_output;
config_value_t value_output;
config_rs485_t rs485_output;

boolean has_value = false;
boolean has_pixels = false;
boolean has_rs485 = false;

int configOffset = -1;

PixelUtil pixels;

RS485Socket rs485;
#define SEND_BUFFER_SIZE 64 // The data size for transmission buffers
byte rs485_data_buffer[RS485_BUFFER_TOTAL(SEND_BUFFER_SIZE)];

void setup() {
  Serial.begin(9600);

  DEBUG2_PRINTLN("***** HMTL Bringup *****");

  int32_t outputs_found = hmtl_setup(&config, readoutputs,
                                     outputs, NULL, HMTL_MAX_OUTPUTS,
                                     &rs485, 
                                     NULL,
                                     &pixels, 
                                     NULL, // MPR121
                                     &rgb_output, // RGB
                                     &value_output, // Value
                                     &configOffset);

  DEBUG4_VALUE("Config size:", configOffset - HMTL_CONFIG_ADDR);
  DEBUG4_VALUELN(" end:", configOffset);
  DEBUG4_COMMAND(hmtl_print_config(&config, outputs));

  /* Setup the RS485 connection if one is configured */
  if (outputs_found & (1 << HMTL_OUTPUT_RS485)) {
    rs485.setup();
    rs485.initBuffer(rs485_data_buffer, SEND_BUFFER_SIZE);
    has_rs485 = true;
  }

  if (outputs_found & (1 << HMTL_OUTPUT_VALUE)) {
    has_value = true;
  }

  if (outputs_found & (1 << HMTL_OUTPUT_PIXELS)) {
    for (unsigned int i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelRGB(i, 0, 0, 0);
    }
    pixels.update();
    has_pixels = true;
  }

  /* ***** Triangle Light Specific Code ***** */
  initializePins();    /* Setup the sensors */

}

#define PERIOD 1000
unsigned long last_change = 0;
int cycle = 0;

void loop() {
  unsigned long now = millis();

  /* Handle the triangle sensors */
  update_sensors();

  static int prev_button_value = 0;
  int button_value = get_button_value();
  if (prev_button_value != button_value) {
    prev_button_value = button_value;
  }

  static uint16_t prev_pot_value = 0;
  uint16_t pot_value = get_pot_value();
  if (abs(pot_value - prev_pot_value) > 5) {
    prev_pot_value = pot_value;
  }

  static uint16_t prev_photo_value = 0;
  uint16_t photo_value = get_photo_value();
  if (abs(photo_value - prev_photo_value) > 5) {
    prev_photo_value = photo_value;
  }

  /*
   * Change the display mode periodically
   */
  if (now - last_change > PERIOD) {

    DEBUG1_VALUE("", cycle);
    DEBUG1_VALUE("- button:", button_value);
    DEBUG1_VALUE(" pot:", pot_value);
    DEBUG1_PRINT(" ");
    DEBUG1_VALUE(" photo:", photo_value);
    DEBUG1_PRINT(" ");

    // Set LED colors
    switch (cycle % 4) {
      case 0: {
        DEBUG1_PRINT("White");
        if (has_value) digitalWrite(value_output.pin, HIGH);
        digitalWrite(rgb_output.pins[0], HIGH);
        digitalWrite(rgb_output.pins[1], HIGH);
        digitalWrite(rgb_output.pins[2], HIGH);
        if (has_pixels) {
          for (unsigned int i=0; i < pixels.numPixels(); i++) 
            pixels.setPixelRGB(i, 255, 255, 255);  
          pixels.update();
        }
        break;
      }

      case 1: {
        DEBUG1_PRINT("Red  ");
        if (has_value) digitalWrite(value_output.pin, LOW);
        digitalWrite(rgb_output.pins[0], HIGH);
        digitalWrite(rgb_output.pins[1], LOW);
        digitalWrite(rgb_output.pins[2], LOW);
        if (has_pixels) {
          for (unsigned int i=0; i < pixels.numPixels(); i++) 
            pixels.setPixelRGB(i, 255, 0, 0);  
          pixels.update();
        }
        break;
      }

      case 2: {
        DEBUG1_PRINT("Green");
        digitalWrite(rgb_output.pins[0], LOW);
        digitalWrite(rgb_output.pins[1], HIGH);
        digitalWrite(rgb_output.pins[2], LOW);
        if (has_pixels) {
          for (unsigned int i=0; i < pixels.numPixels(); i++) 
            pixels.setPixelRGB(i, 0, 255, 0);  
          pixels.update();
        }
        break;
      }

      case 3: {
        DEBUG1_PRINT("Blue ");
        digitalWrite(rgb_output.pins[0], LOW);
        digitalWrite(rgb_output.pins[1], LOW);
        digitalWrite(rgb_output.pins[2], HIGH);
        if (has_pixels) {
          for (unsigned int i=0; i < pixels.numPixels(); i++) 
            pixels.setPixelRGB(i, 0, 0, 255);  
          pixels.update();
        }

        if (has_rs485) {
          // Broadcast a message
          DEBUG1_PRINT(" - Sending rs485");
          hmtl_send_cancel(&rs485, rs485.send_buffer, rs485.send_data_size, 
                           SOCKET_ADDR_ANY, 0);
        }

        break;
      }
    }

    cycle++;
    last_change = now;
  }

  DEBUG_PRINT_END();

  if (has_rs485) {
    /*
     * Check for data over RS485
     */
    unsigned int msglen;
    msg_hdr_t *msg = hmtl_rs485_getmsg(&rs485, &msglen, config.address);
    if (msg != NULL) {
      DEBUG1_VALUE("Recieved rs485 msg len:", msglen);
      DEBUG1_VALUE(" src:", RS485_SOURCE_FROM_DATA(msg));
      DEBUG1_VALUE(" dst:", RS485_ADDRESS_FROM_DATA(msg));
      DEBUG1_VALUE(" len:", msg->length);
      DEBUG1_VALUE(" type:", msg->type);
      DEBUG1_HEXVAL(" flags:0x", msg->flags);
      DEBUG1_PRINT(" data:");
      DEBUG1_COMMAND(
                     print_hex_string((byte *)msg, msglen)
                     );
      DEBUG_PRINT_END();
      
      // TODO: Move code from command cli into library
    }

  }
}
