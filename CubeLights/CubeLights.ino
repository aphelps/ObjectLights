/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
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

#define CONFIG_ENABLED
#ifdef CONFIG_ENABLED
#define CUBE_MAX_OUTPUTS 4

void readConfig() {
  config_hdr_t config;
  output_hdr_t *outputs[CUBE_MAX_OUTPUTS];
  config_max_t readoutputs[CUBE_MAX_OUTPUTS];
  int offset;

  /* Attempt to read the configuration */
  offset = hmtl_read_config(&config, readoutputs, CUBE_MAX_OUTPUTS);
  if (offset < 0) {
    hmtl_default_config(&config);
    DEBUG_PRINTLN(DEBUG_LOW, "ERROR: Using default config");
  }
 if (config.num_outputs > CUBE_MAX_OUTPUTS) {
    DEBUG_VALUELN(0, "Too many outputs:", config.num_outputs);
    DEBUG_ERR_STATE(DEBUG_ERR_INVALID);
  }
  for (int i = 0; i < config.num_outputs; i++) {
    outputs[i] = (output_hdr_t *)&readoutputs[i];
  }
  DEBUG_COMMAND(DEBUG_HIGH, hmtl_print_config(&config, outputs));

  // XXX: Perform output validation, check that pins are used only once, etc

  /* Initialize the outputs */
  for (int i = 0; i < config.num_outputs; i++) {
    void *data = NULL;
    switch (((output_hdr_t *)outputs[i])->type) {
    case HMTL_OUTPUT_PIXELS: data = &pixels; break;
    case HMTL_OUTPUT_MPR121: data = &touch_sensor; break;
    case HMTL_OUTPUT_RS485: data = &rs485; break;
    }
    hmtl_setup_output((output_hdr_t *)outputs[i], data);
  }

  offset = readConfiguration(squares, numSquares, offset);
}
#endif

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

  /* Generate the geometry */
  squares = buildCube(&numSquares, numLeds, FIRST_LED);
  DEBUG_VALUELN(DEBUG_LOW, "* Inited with numSquares:", numSquares);

#ifdef CONFIG_ENABLED
  Wire.begin();
  readConfig();

  /* Setup the external connection */
  initializeConnect();
#else
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
