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
#include <Adafruit_WS2801.h>
#include <SoftwareSerial.h>
#include <RS485_non_blocking.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include "GeneralUtils.h"
#include "PixelUtil.h"
#include "RS485Utils.h"
#include "MPR121.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"

#include "CubeConfig.h"
#include "CubeConfiguration.h"
#include "SquareStructure.h"
#include "CubeLights.h"


#define DEBUG_LED 13

/* Auto update build number */
#define CUBE_LIGHT_BUILD 14 // %META INCR

pattern_args_t modeConfigs[MAX_MODES] = {
  {
    pixel_color(0, 0, 0), // bgColor
    pixel_color(0xFF, 0xFF, 0xFF), // fgColor
    0, // next_time
    0, // periodms
    0 // data
  },
  {
    pixel_color(0, 0, 0), // bgColor
    pixel_color(0xFF, 0xFF, 0xFF), // fgColor
    0, // next_time
    0, // periodms
    0 // data
  },
  {
    pixel_color(0, 0, 0), // bgColor
    pixel_color(0xFF, 0xFF, 0xFF), // fgColor
    0, // next_time
    0, // periodms
    0 // data
  }
};

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  Serial.begin(9600);
  DEBUG_PRINTLN(DEBUG_LOW, "*** CubeLights Initializing ***");
  DEBUG_PRINTLN(DEBUG_LOW, "* Baud is 9600");
  //Serial.print("Set baud rate to 115200");
  //Serial.begin(115200);

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(0) + analogRead(2) + micros());

#ifdef CONFIG_ENABLED
  Wire.begin();
  readHMTLConfiguration();

  /* Setup the external connection */
  initializeConnect();
#else
  /* Generate the geometry */
  int numLeds = 45 + FIRST_LED;
  int numSquares = NUM_SQUARES;
  squares = buildCube(&numSquares, numLeds, FIRST_LED);
  DEBUG_VALUELN(DEBUG_LOW, "* Inited with NUM_SQUARES:", numSquares);

  pixels = PixelUtil(numLeds, 12, 8);
  sensor_cap_init(); /* Initialize the capacitive sensors */
#endif

  /* Setup the sensors */
  initializePins();

  DEBUG_VALUE(DEBUG_LOW, "* Setup complete for CUBE_NUMBER=", CUBE_NUMBER);
  DEBUG_VALUELN(DEBUG_LOW, " Build=", CUBE_LIGHT_BUILD);
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
  sensor_range();

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

  DEBUG_COMMAND(DEBUG_TRACE, // Flash the debug LED
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
