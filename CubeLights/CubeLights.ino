/*******************************************************************************
 * Copyright: 2013, Adam Phelps
 * 
 * XXX: Put a license here
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
#include "MPR121.h"

#define SETUP_STATE 0 // Used during structure configuration
#define DEBUG_LED 13

int numLeds = 45;
PixelUtil pixels;

int numSquares = 6;
Square *squares;

square_mode_t modeFunctions[] = {
  squaresAllOn,          // 0
  squaresTestPattern,    // 1
  squaresSetupPattern,   // 2
  squaresRandomNeighbor, // 3
  squaresCyclePattern,   // 4
  squaresCirclePattern,  // 5
  squaresFadeCycle       // 6
};
#define NUM_MODES (sizeof (modeFunctions) / sizeof (cube_mode_t))

uint16_t modePeriods[] = {
  1000,
  1000,
  500,
  500,
  500,
  500,
  1
};

pattern_args_t patternConfig = {
  pixel_color(0, 0, 0), // bgColor
  pixel_color(0xF, 0xF, 0xF) // fgColor
};

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  Serial.begin(9600);
  DEBUG_PRINT(DEBUG_HIGH, "Baud is 9600");
  //Serial.print("Set baud rate to 115200");
  //Serial.begin(115200);

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(3));

  pixels = PixelUtil(numLeds, 12, 8);

  /* Setup the sensors */
  initializePins();
  //sensor_cap_init(); /* Initialize the capacitive sensors */

  /* Generate the geometry */
  squares = buildCube(&numSquares, numLeds);
  DEBUG_VALUELN(DEBUG_HIGH, "Inited with numSquares:", numSquares);

  DEBUG_PRINTLN(DEBUG_MID, "Setup complete");
}



/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
#if SETUP_STATE == 1
  setupMode(); 
  return;
#endif

  static byte prev_mode = -1;
  byte mode = 0;

  mode = 4; //getButtonValue() % NUM_MODES;
  if (mode != prev_mode) {
    DEBUG_VALUE(DEBUG_HIGH, "mode=", mode);
    DEBUG_MEMORY(DEBUG_HIGH);
  }

  /* Check for update of light sensor value */
  sensor_photo();

  /* Run the current mode and update the squares */
  modeFunctions[mode](squares, numSquares, modePeriods[mode],
		      prev_mode != mode, &patternConfig);
  updateSquarePixels(squares, numSquares, &pixels);
  prev_mode = mode;

  DEBUG_VALUELN(DEBUG_HIGH, "time:", millis());

  DEBUG_COMMAND(DEBUG_TRACE,
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
