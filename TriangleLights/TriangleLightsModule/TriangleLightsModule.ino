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

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL DEBUG_HIGH
#endif
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "Socket.h"
#include "RS485Utils.h"
#include "XBeeSocket.h"
#include "MPR121.h" // XXX This needs to go
#include "SerialCLI.h"

#include "NewPing.h" // XXX This also needs to go, can be compiled out

#include "ObjectConfiguration.h"
#include "TriangleStructure.h"
#include "TriangleLights.h"
#include "TriangleLightsModes.h"


extern SerialCLI serialcli;

PixelUtil pixels;


RS485Socket rs485;

#define SEND_BUFFER_SIZE 64 // The data size for transmission buffers
byte rs485_data_buffer[RS485_BUFFER_TOTAL(SEND_BUFFER_SIZE)];

#define MAX_SOCKETS 1
Socket *sockets[MAX_SOCKETS] = { NULL };


int numTriangles = 0;
Triangle *triangles;

#define SETUP_STATE 0 // Used during structure configuration

#define DEBUG_LED 13

/* Module configuration */
#define MAX_OUTPUTS 7
config_hdr_t config;
output_hdr_t *outputs[MAX_OUTPUTS];
config_max_t readoutputs[MAX_OUTPUTS];
void *objects[MAX_OUTPUTS];


#define MODE_PERIOD 50
triangle_mode_t modeFunctions[] = {
  //trianglesSetAll,
  // trianglesVertexMergeFade,
  //  trianglesVertexMerge,
  //trianglesVertexShift,
  trianglesSnake2,
  //trianglesLooping,
  //trianglesCircle,
  //  trianglesCircleCorner2,
  //  trianglesRandomNeighbor,
  //trianglesLifePattern,
  //  trianglesLifePattern2,
  //  trianglesBuildup,
  //trianglesStaticNoise,
//  trianglesSwapPattern
//  trianglesTestPattern,
//  trianglesCircleCorner,
};
#define NUM_MODES (sizeof (modeFunctions) / sizeof (triangle_mode_t))

uint16_t modePeriods[] = {
  //1000,
  // MODE_PERIOD,
  // MODE_PERIOD,
  //MODE_PERIOD * 2,
  MODE_PERIOD,
  //MODE_PERIOD,
  // MODE_PERIOD,
  // MODE_PERIOD,
  // MODE_PERIOD,
  // 500,
  // 500,
  // MODE_PERIOD,
  //MODE_PERIOD,
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
  Serial.begin(57600);

  if (NUM_PERIODS != NUM_MODES) {
    DEBUG_ERR("NUM_MODES != NUM_PERIODS");
    DEBUG_ERR_STATE(1);
  }

  DEBUG4_PRINTLN("*** TriangleLights Initializing ***");

  pinMode(DEBUG_LED, OUTPUT);

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(3) + analogRead(4) + micros());

  //Wire.begin(); // Needed for MPR121
  int configOffset = readHMTLConfiguration(&config,
                                           outputs, readoutputs, objects,
                                           MAX_OUTPUTS,
                                           &pixels, &rs485, NULL);


  /* Setup the RS485 connection */
  byte num_sockets = 0;
  rs485.setup();
  rs485.initBuffer(rs485_data_buffer, SEND_BUFFER_SIZE);
  sockets[num_sockets++] = &rs485;

  init_modes(sockets, num_sockets);

  /* Setup the sensors */
  initializePins();

  /* Read the triangle structure from EEPROM */
  readTriangleStructure(configOffset, 
                        &triangles,
                        &numTriangles);

  DEBUG2_VALUELN("Inited with numTriangles:", numTriangles);

  // Send the ready signal to the serial port
  Serial.println(F(HMTL_READY));
}

void loop() {

  //serialcli.checkSerial();

  static byte prev_mode = -1;
  byte mode;

  // TODO: This should be changing the program
  mode = get_button_value() % NUM_MODES;
  if (mode != prev_mode) {
    DEBUG4_VALUELN("mode=", mode);
    DEBUG_MEMORY(DEBUG_HIGH);
  }
  prev_mode = mode;

  /* Check for update of the sensor values */
  update_sensors();

  /* When nothing else is working you can always blink and LED */
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

  /*
   * Check for messages and handle output states
   */
  boolean updated = messages_and_modes();

}