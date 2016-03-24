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
#include "SerialCLI.h"

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
#ifndef MAX_OUTPUTS
  #define MAX_OUTPUTS 7
  #warning Using default MAX_OUTPUTS value!
#endif
config_hdr_t config;
output_hdr_t *outputs[MAX_OUTPUTS];
config_max_t readoutputs[MAX_OUTPUTS]; // TODO: Keeping them all like this is BIG
void *objects[MAX_OUTPUTS];


void setup()
{
  Serial.begin(57600);

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

  // TODO: How to read HTML commands and CLI commands at the same time
  // serialcli.checkSerial();

  // TODO: This should be changing the program
  update_mode_from_button();

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