/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Main sketch for running TriangleLights
 ******************************************************************************/

#include <Arduino.h>
#include "SPI.h"
#include "Wire.h" // XXX: This also needs to go
#include "FastLED.h"
#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "Socket.h"
#include "RS485Utils.h"
#include "XBeeSocket.h"
#include "MPR121.h" // XXX This needs to go

#include "NewPing.h" // XXX This also needs to go, can be compiled out

#include "ObjectConfiguration.h"
#include "TriangleStructure.h"
#include "TriangleLights.h"

PixelUtil pixels;
RS485Socket rs485;

int numTriangles = 0;
Triangle *triangles;

#define SETUP_STATE 0 // Used during structure configuration

#define DEBUG_LED 13

#define MODE_PERIOD 50
triangle_mode_t modeFunctions[] = {
  trianglesSetAll,
  trianglesVertexMergeFade,
  //  trianglesVertexMerge,
  trianglesVertexShift,
  trianglesSnake2,
  trianglesLooping,
  //trianglesCircle,
  //  trianglesCircleCorner2,
  //  trianglesRandomNeighbor,
  //trianglesLifePattern,
  //  trianglesLifePattern2,
  //  trianglesBuildup,
  trianglesStaticNoise,
//  trianglesSwapPattern
//  trianglesTestPattern,
//  trianglesCircleCorner,
};
#define NUM_MODES (sizeof (modeFunctions) / sizeof (triangle_mode_t))

uint16_t modePeriods[] = {
  1000,
  MODE_PERIOD,
  //  MODE_PERIOD,
  MODE_PERIOD * 2,
  MODE_PERIOD,
  MODE_PERIOD,
  //MODE_PERIOD,
  //  MODE_PERIOD,
  //  MODE_PERIOD,
  // 500,
  //  500,
  //  MODE_PERIOD,
  MODE_PERIOD,
//  10
//  500,
//  MODE_PERIOD,
};
#define NUM_PERIODS (sizeof (modePeriods) / sizeof (uint16_t))


pattern_args_t patternConfig = {
  pixel_color(0, 0, 0), // bgColor
  pixel_color(0xFF, 0xFF, 0xFF) // fgColor
};

void setup()
{
  Serial.begin(9600);

  if (NUM_PERIODS != NUM_MODES) {
    DEBUG_ERR("NUM_MODES != NUM_PERIODS");
    DEBUG_ERR_STATE(1);
  }

  DEBUG4_PRINTLN("*** TriangleLights Initializing ***");

  pinMode(DEBUG_LED, OUTPUT);

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(3) + analogRead(4) + micros());

  //Wire.begin(); // Needed for MPR121
 #define MAX_OUTPUTS 7
  config_hdr_t config;
  output_hdr_t *outputs[MAX_OUTPUTS];
  config_max_t readoutputs[MAX_OUTPUTS];
  int configOffset = readHMTLConfiguration(&config, 
                                           outputs, readoutputs, NULL, MAX_OUTPUTS,
                                           &pixels, &rs485, NULL);

  /* Setup the sensors */
  initializePins();

  // TODO: WHY DOES THIS HAVE TO BE HERE??? Get rid of it somehow?
  triangles = buildIcosohedron(&numTriangles, pixels.numPixels());

  /* Read the triangle structure from EEPROM */
  readTriangleStructure(configOffset, 
                        &triangles,
                        &numTriangles);

  DEBUG2_VALUELN("Inited with numTriangles:", numTriangles);
}

void loop() {

  static byte prev_mode = -1;
  byte mode;

  mode = get_button_value() % NUM_MODES;
  if (mode != prev_mode) {
    DEBUG4_VALUELN("mode=", mode);
    DEBUG_MEMORY(DEBUG_HIGH);
  }

  /* Check for update of the sensor values */
  update_sensors();

  /* Run the current mode and update the triangles */
  modeFunctions[mode](triangles, numTriangles, modePeriods[mode],
		      prev_mode != mode, &patternConfig);
  updateTrianglePixels(triangles, numTriangles, &pixels);
  prev_mode = mode;

  DEBUG_COMMAND(DEBUG_HIGH,
		static unsigned long next_millis = 0;
		if (millis() > next_millis) {
		  if (next_millis % 2 == 0) {
		    digitalWrite(DEBUG_LED, HIGH);
		  } else {
		    digitalWrite(DEBUG_LED, LOW);
		  }
		  next_millis += 251;
		}
		);
}

void serialEvent() {
  cliRead();
}
