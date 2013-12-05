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

#include "CubeLights.h"
#include "MPR121.h"

mode_function_t modeFunctions[] = {
  mode_set_all,          // MODE_SET_ALL
  mode_swap_one,         // MODE_SWAP_ONE
  mode_fade_one,         // MODE_FADE_ONE
  mode_count_up,         // MODE_COUNT_UP
  mode_flash_ordered,    // MODE_FLASH_ORDERED
  mode_random_fades,     // MODE_RANDOM_FADES
  mode_sense_distance,   // MODE_SENSE_DISTANCE
};

void * modeArguments[] = {
  (void *)MAX_VALUE,    // MODE_SET_ALL
  NULL,                 // MODE_SWAP_ONE
  NULL,                 // MODE_FADE_ONE
  NULL,                 // MODE_COUNT_UP
  NULL,                 // MODE_FLASH_ORDERED
  NULL,                 // MODE_RANDOM_FADES
  NULL,                 // MODE_SENSE_DISTANCE
};


#define SETUP_STATE 0 // Used during structure configuration
#define DEBUG_LED 13

int numLeds = 45;
PixelUtil pixels;


/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  Serial.begin(9600);
  DEBUG_PRINT(DEBUG_HIGH, "Baud is 9600");
  //Serial.print("Set baud rate to 115200");
  //Serial.begin(115200);

  randomSeed(analogRead(0));

  //sensor_cap_init(); /* Initialize the capacitive sensors */

  pixels = PixelUtil(numLeds, 12, 8);

  /* Turn on input pullup on analog photo pin */
  digitalWrite(PHOTO_PIN, HIGH); 

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

  pixels.patternOne(50);
  pixels.update();
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
