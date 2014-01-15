/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 ******************************************************************************/

#include <Arduino.h>
#include <NewPing.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_WS2801.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include "GeneralUtils.h"
#include "PixelUtil.h"

#include "SquareStructure.h"
#include "CubeLights.h"
#include "CubeConfig.h"
#include "MPR121.h"

#define DEBUG_LED 13

int numLeds = 45;
PixelUtil pixels;

int numSquares = 6;
Square *squares;

pattern_args_t modeConfig = {
  pixel_color(0, 0, 0), // bgColor
  pixel_color(0xFF, 0xFF, 0xFF) // fgColor
};

pattern_args_t followupConfig = {
  pixel_color(0, 0, 0), // bgColor
  pixel_color(0xFF, 0x00, 0x00) // fgColor
};

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  Serial.begin(9600);
  DEBUG_PRINTLN(DEBUG_HIGH, "Baud is 9600");
  //Serial.print("Set baud rate to 115200");
  //Serial.begin(115200);

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(3));

  pixels = PixelUtil(numLeds, 12, 8);

  /* Setup the sensors */
  initializePins();
  sensor_cap_init(); /* Initialize the capacitive sensors */

  /* Generate the geometry */
  squares = buildCube(&numSquares, numLeds);
  DEBUG_VALUELN(DEBUG_HIGH, "Inited with numSquares:", numSquares);

  DEBUG_VALUELN(DEBUG_MID, "Setup complete for CUBE_NUMBER=", CUBE_NUMBER);
}



/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
  static byte prev_mode = -1;
  byte mode = 0, followup = 0;

  /* Check the sensor values */
  //sensor_photo();
  sensor_cap();
  sensor_range();

  handle_sensors();

  /* Run the current mode and update the squares */
  mode = get_current_mode();
  modeFunctions[mode](squares, numSquares, modePeriods[mode],
		      prev_mode != mode, &modeConfig);

  /* Run any follup function */
  followup = get_current_followup();
  if (followup != (byte)-1) {
    modeFunctions[followup](squares, numSquares, modePeriods[followup],
				prev_mode != mode, &followupConfig);  
  }

  /* Send any changes */
  updateSquarePixels(squares, numSquares, &pixels);

  prev_mode = mode;

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
