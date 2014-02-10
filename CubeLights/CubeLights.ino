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

int numLeds = 45 + FIRST_LED;

pattern_args_t modeConfig = {
  pixel_color(0, 0, 0), // bgColor
  pixel_color(0xFF, 0xFF, 0xFF), // fgColor
  0, // next_time
  0 // data
};

pattern_args_t followupConfig = {
  pixel_color(0, 0, 0), // bgColor
  pixel_color(0xFF, 0x00, 0x00), // fgColor
  0, // next_time
  CUBE_TOP // data
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
  randomSeed(analogRead(3));

#ifdef CONFIG_ENABLED
  Wire.begin();
  readHMTLConfiguration();

  /* Setup the external connection */
  initializeConnect();
#else
  /* Generate the geometry */
  squares = buildCube(&NUM_SQUARES, numLeds, FIRST_LED);
  DEBUG_VALUELN(DEBUG_LOW, "* Inited with NUM_SQUARES:", NUM_SQUARES);

  pixels = PixelUtil(numLeds, 12, 8);
  sensor_cap_init(); /* Initialize the capacitive sensors */
#endif

  /* Setup the sensors */
  initializePins();

  DEBUG_VALUELN(DEBUG_LOW, "* Setup complete for CUBE_NUMBER=", CUBE_NUMBER);
  DEBUG_MEMORY(DEBUG_HIGH);
}



/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
  static byte prev_mode = -1, prev_followup = -1;
  byte mode = 0, followup = 0;

  /* Check the sensor values */
  //sensor_photo();
  sensor_cap();
  sensor_range();

  handle_sensors();

  /* Run the current mode and update the squares */
  mode = get_current_mode();
  modeConfig.periodms = modePeriods[mode];
  modeFunctions[mode](squares, NUM_SQUARES,
		      prev_mode != mode, &modeConfig);

  /* Run any follup function */
  followup = get_current_followup();
  if (followup != (byte)-1) {
    followupConfig.periodms = modePeriods[followup];
    modeFunctions[followup](squares, NUM_SQUARES,
			    prev_followup != followup, &followupConfig);
  }

  /* Send any changes */
  updateSquarePixels(squares, NUM_SQUARES, &pixels);

  prev_mode = mode;
  prev_followup = followup;

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
