/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#include <Arduino.h>
#include "EEPROM.h"
#include <NewPing.h>
#include <Wire.h>
#include <SPI.h>
#include <FastLED.h>
#include <SoftwareSerial.h>
#include <RS485_non_blocking.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include "GeneralUtils.h"
#include "PixelUtil.h"
#include "Socket.h"
#include "RS485Utils.h"
#include "MPR121.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "HMTLMessaging.h"

#include "CubeConfig.h"
#include "CubeConfiguration.h"
#include "SquareStructure.h"
#include "CubeLights.h"

#include "Geometry.h"

#define DEBUG_LED 13

/* Auto update build number */
#define CUBE_LIGHT_BUILD 25 // %META INCR

pattern_args_t modeConfigs[MAX_MODES] = {
  {
    pixel_color(0, 0, 0), // bgColor
    pixel_color(0xFF, 0xFF, 0xFF), // fgColor
    0, // next_time
    0, // periodms
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}} // data
  },
  {
    pixel_color(0, 0, 0), // bgColor
    pixel_color(0xFF, 0xFF, 0xFF), // fgColor
    0, // next_time
    0, // periodms
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}} // data
  },
  {
    pixel_color(0, 0, 0), // bgColor
    pixel_color(0xFF, 0xFF, 0xFF), // fgColor
    0, // next_time
    0, // periodms
    {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}} // data
  }
};

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  Serial.begin(9600);
  DEBUG2_PRINTLN("*** CubeLights Initializing ***");
  DEBUG2_PRINTLN("* Baud is 9600");

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(0) + analogRead(2) + micros());

  Wire.begin();


 #define MAX_OUTPUTS 4
  config_hdr_t config;
  output_hdr_t *outputs[MAX_OUTPUTS];
  config_max_t readoutputs[MAX_OUTPUTS];
  readHMTLConfiguration(&config, outputs, readoutputs, MAX_OUTPUTS);

  /* Setup the external connection */
  initializeConnect();

  /* Setup the sensors */
  initializePins();

  DEBUG2_VALUE("* Setup complete for CUBE_NUMBER=", CUBE_NUMBER);
  DEBUG2_VALUELN(" Build=", CUBE_LIGHT_BUILD);
  DEBUG_MEMORY(DEBUG_HIGH);
}



/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
  /* Check the sensor values */
  //sensor_photo();
  sensor_cap();
  //  sensor_range();

  handle_sensors();

  for (int i = 0; i < MAX_MODES; i++) {
    byte mode = get_current_mode(i);
    if (mode != MODE_NONE) {
      modeConfigs[i].periodms = modePeriods[mode]; // XXX - Get this from elsewhere?
      modeFunctions[mode](squares, NUM_SQUARES, &modeConfigs[i]);
    }
  }

  /* Send any changes */
  updateSquarePixels(squares, NUM_SQUARES, &pixels);

  DEBUG5_COMMAND(// Flash the debug LED
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
