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

  squares[0].setColor(255, 0, 0);   // R
  squares[1].setColor(0, 255, 0);   // G
  squares[2].setColor(0, 0, 255);   // B
  squares[3].setColor(255, 255, 0); // Y
  squares[4].setColor(255, 0, 255); // Purple
  updateSquarePixels(squares, numSquares, &pixels);

#if 0
  mode = getButtonValue() % NUM_MODES;
  if (mode != prev_mode) {
    DEBUG_VALUE(DEBUG_HIGH, "mode=", mode);
    DEBUG_MEMORY(DEBUG_HIGH);
  }

  /* Check for update of light sensor value */
  sensor_photo();

  /* Run the current mode and update the triangles */
  modeFunctions[mode](triangles, numTriangles, modePeriods[mode],
		      prev_mode != mode, &patternConfig);
  updateTrianglePixels(triangles, numTriangles, &pixels);
  prev_mode = mode;
#else

#if 0
  static unsigned long next_time = millis();

  if (millis() > next_time) {
    prev_mode++;
    next_time = next_time + random(10, 100);

    for (int i = 0; i < numLeds; i++) {
      if (prev_mode % 2 == 0) {
	pixels.setPixelRGB(i, 0, 0, 0);
      } else {
	pixels.setPixelRGB(i, 255, 255, 255);
      }
    }
    DEBUG_VALUELN(DEBUG_HIGH, "mode=", mode);
  }
#endif

  //  pixels.patternOne(50);
  //pixels.update();
#endif

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
