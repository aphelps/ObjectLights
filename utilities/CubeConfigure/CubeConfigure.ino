/*******************************************************************************
 * Write out a CubeLight configuration and provide a CLI for establishing the
 * individual faces and LEDs of the Cube.
 *
 * Author: Adam Phelps
 * License: Creative Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "Adafruit_WS2801.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "Wire.h"
#include "MPR121.h"
#include "SerialCLI.h"
#include "RS485Utils.h"

#include "SquareStructure.h"
#include "CubeLights.h"
#include "CubeConfig.h"
#include "CubeConfiguration.h"

boolean wrote_config = false;

#define PIN_DEBUG_LED 13

int numSquares = 6;

#define MAX_OUTPUTS 4
config_hdr_t config;
output_hdr_t *outputs[MAX_OUTPUTS];

config_value_t val_output, val_output2, val_output3, val_output4;
config_rgb_t rgb_output, rgb_output2;
config_pixels_t pixel_output;
config_mpr121_t mpr121_output;
config_rs485_t rs485_output;

SerialCLI serialcli(128, cliHandler);

config_hdr_t readconfig;
config_max_t readoutputs[MAX_OUTPUTS];

int configOffset = -1;

// XXX: These should probably come from libraries
RS485Socket rs485;
MPR121 touch_sensor;
uint16_t my_address;

void setup() 
{
  Serial.begin(9600);

  DEBUG_PRINTLN(DEBUG_LOW, "*** CubeConfigure started ***");

  /* Read the current configuration from EEProm */
  readHMTLConfiguration();
  clearSquares();

  pinMode(PIN_DEBUG_LED, OUTPUT);

  DEBUG_VALUELN(DEBUG_LOW, "Configure initialized.  End address=",
		configOffset);
  DEBUG_MEMORY(DEBUG_HIGH);
}

boolean output_data = false;
void loop() 
{
  if (!output_data) {
    DEBUG_VALUE(0, "My address:", config.address);
    DEBUG_VALUELN(0, " Was config written: ", wrote_config);
    hmtl_print_config(&config, outputs);
    output_data = true;
  }

  serialcli.checkSerial();

  //  for (int tri = 0; tri < numSquares; tri++) {
  //    squares[tri].setColor(pixel_color(255, 0, 0));
  //    squares[tri].mark = 0;
  //  }

  blink_value(PIN_DEBUG_LED, config.address, 250, 4);
  delay(10);
}

byte currentPixel = 0;
byte current_face = -1;
byte current_led = -1;

/*
 * CLI Handler to setup the cube geometry
 *
 * l <face> <led> - Toggle the state of an LED on the indicated face
 * s <face> <led> - Set the geometry of the current pixel
 * c - Display current pixel
 * C <led> - Set current pixel
 * n - Advance to the next pixel
 * p - Return to the previous pixel
 * f <face> - Light the indicated face
 * t <sensor> <touch> <release> - Set cap sensor thresholds
 * write - Write out the configuration
 */
void cliHandler(char **tokens, byte numtokens) {

  switch (tokens[0][0]) {
    case 'l': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);
      DEBUG_VALUE(DEBUG_LOW, "Light Face:", face);
      DEBUG_VALUE(DEBUG_LOW, " LED:", led);
      DEBUG_VALUELN(DEBUG_LOW, " Pixel:", squares[face].leds[led].pixel);
      setSquareLED(face, led, pixel_color(255, 255, 255));
      break;
    }

    case 'f': {
      if (numtokens < 2) return;
      byte face = atoi(tokens[1]);
      DEBUG_VALUELN(DEBUG_LOW, "Light Face:", face);
      setSquareFace(face, pixel_color(255, 255, 255), true);
      break;
    }


    case 's': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);
      squares[face].setLedPixel(led, currentPixel);
      setSquareLED(face, led, pixel_color(255, 0, 0));
      DEBUG_VALUE(DEBUG_LOW, "Set Face:", face);
      DEBUG_VALUE(DEBUG_LOW, " LED:", led);
      DEBUG_VALUELN(DEBUG_LOW, " Pixel:", currentPixel);
      break;
    }

    case 'c': {
      clearPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "current:", currentPixel);
      break;
    }

  case 'C': {
      if (numtokens < 2) return;
      byte led = atoi(tokens[1]);


      currentPixel = led;
      clearPixels();
      pixels.setPixelRGB(currentPixel, 0, 255, 0);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "set current:", currentPixel);
      break;
    }

    case 'n': {
      clearPixels();
      currentPixel = (currentPixel + 1) % pixels.numPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "current:", currentPixel);
      break;
    }

    case 'p': {
      clearPixels();
      currentPixel = (currentPixel + pixels.numPixels() - 1) % pixels.numPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "next:", currentPixel);
      break;
    }

  case 't': {
    // Set a sensor's threshold value
    if (numtokens < 4) return;
    byte sensor = atoi(tokens[1]);
    byte touch = atoi(tokens[2]);
    byte release = atoi(tokens[3]);

    if ((sensor != 0) && (sensor != 1)) {
      DEBUG_ERR("Sensor must be 0 or 1");
      return;
    }
    if ((touch < 1) || (touch > 15)) {
      DEBUG_ERR("Touch value outside valid range");
      return;
    }
    if ((release < 1) || (release > 15)) {
      DEBUG_ERR("Release value outside valid range");
      return;
    }

    // Locate the MPR121 sensor
    config_mpr121_t *mpr121 = NULL;
    for (int i = 0; i < config.num_outputs; i++) {
      if (outputs[i]->type == HMTL_OUTPUT_MPR121) {
	mpr121 = (config_mpr121_t *)outputs[i];
      }
    }
    if (mpr121 == NULL) {
      DEBUG_ERR("Failed to find the MPR121 config");
      return;
    }

    if (sensor == 0) {
        mpr121->thresholds[CAP_SENSOR_1] = (touch) | (release << 4);
    } else if (sensor == 1) {
        mpr121->thresholds[CAP_SENSOR_2] = (touch) | (release << 4);
    }
    DEBUG_VALUE(DEBUG_LOW, "Set sensor ", sensor);
    DEBUG_VALUE(DEBUG_LOW, " touch=", touch);
    DEBUG_VALUELN(DEBUG_LOW, " release=", release);

    break;
  }

  case 'N': {
    if (numtokens < 2) return;
    byte numleds = atoi(tokens[1]);

    // Locate the pixels
    config_pixels_t *pixels = NULL;
    for (int i = 0; i < config.num_outputs; i++) {
      if (outputs[i]->type == HMTL_OUTPUT_PIXELS) {
        pixels = (config_pixels_t *)outputs[i];
      }
    }
    if (pixels == NULL) {
      DEBUG_ERR("Failed to find the pixels config");
      return;
    }
    pixels->numPixels = numleds;

    DEBUG_VALUELN(DEBUG_LOW, "Set numpixels to ", numleds);

    break;
  }

  case 'w': {
    if (strcmp(tokens[0], "write") == 0) {
      configOffset = hmtl_write_config(&config, outputs);
      if (configOffset < 0) {
	DEBUG_ERR("Failed to write config");
      }
      writeCubeConfiguration(squares, numSquares, configOffset);
    }
    break;
  }

  }

}

void clearPixels() {
  // current_face = -1;
  for (byte led = 0; led < pixels.numPixels(); led++) {
    pixels.setPixelRGB(led, 0);
  }
  pixels.update();
  for (int face = 0; face < numSquares; face++) {
    squares[face].setColor(0);
  }
  updateSquarePixels(squares, numSquares, &pixels);
}

void setSquareLED(byte face, byte led, uint32_t color) {
  // Turn off the current led and turn on the new one
  squares[current_face].setColor(current_led, 0);
  squares[face].setColor(led, color);
  current_face = face;
  current_led = led;
  updateSquarePixels(squares, numSquares, &pixels);
}

void setSquareFace(byte face, uint32_t color, boolean neighbors) {
  // Turn off the current face and turn on the new one
  for (int f = 0; f < numSquares; f++) {
    squares[f].setColor(0);
  }

  squares[face].setColor(color);
  squares[face].setColor(1, 0);

  current_face = face;

  if (neighbors) {
    squares[face].edges[Square::TOP]->setColor(128, 0, 0);
    squares[face].edges[Square::TOP]->setColor(1, 0, 0, 0);

    squares[face].edges[Square::RIGHT]->setColor(0, 128, 0);
    squares[face].edges[Square::RIGHT]->setColor(1, 0, 0, 0);

    squares[face].edges[Square::BOTTOM]->setColor(0, 0, 128);
    squares[face].edges[Square::BOTTOM]->setColor(1, 0, 0, 0);

    squares[face].edges[Square::LEFT]->setColor(128, 0, 128);
    squares[face].edges[Square::LEFT]->setColor(1, 0, 0, 0);
  }

  updateSquarePixels(squares, numSquares, &pixels);
}

void clearSquares() {
  for (int face = 0; face < numSquares; face++) {
    for (int led = 0; led < Square::NUM_LEDS; led++) {
      squares[face].setLedPixel(led, -1);
    }
  }
}
