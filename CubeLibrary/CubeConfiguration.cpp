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

PixelUtil pixels;
Square *squares;

#define CUBE_MAX_OUTPUTS 4
void readHMTLConfiguration() {
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

  /* Store the configured address */
  my_address = config.address;

  /* Construct the initial cube geometry */
  int numSquares;
  squares = buildCube(&numSquares, pixels.numPixels(), FIRST_LED);

  /* Read in the CubeLight specific configuration */
  offset = readCubeConfiguration(squares, NUM_SQUARES, offset);
}

/*
 * Read in the config of the cube and each square
 */
int readCubeConfiguration(Square *squares, int numSquares, int offset) {
  byte bytes[CONFIG_BUFFER_SZ];

  // Read the overall configuration
  offset = EEPROM_safe_read(offset, bytes, CONFIG_BUFFER_SZ);
  cube_config_t *config = (cube_config_t *)bytes;
  DEBUG_VALUE(DEBUG_LOW, "Read version=", config->version);
  // XXX - What to do with config?

  // Read all squares and initialize them
  for (int face = 0; face < numSquares; face++) {
    offset = EEPROM_safe_read(offset, bytes, CONFIG_BUFFER_SZ);
    if (offset <= 0) {
      DEBUG_ERR("Failed to read squares data");
      break;
    }
    squares[face].fromBytes(bytes, CONFIG_BUFFER_SZ, squares, numSquares);

    DEBUG_VALUE(DEBUG_LOW, " - face=", face);
    DEBUG_VALUE(DEBUG_LOW, " offset=", offset);
    DEBUG_VALUE(DEBUG_LOW, " id=",  squares[face].id);
    DEBUG_VALUE(DEBUG_LOW, " top=", squares[face].edges[Square::TOP]->id);
  }
  DEBUG_PRINT_END();

  return offset;
}

/*
 * Write out the config of the cube and each square
 */
int writeCubeConfiguration(Square *squares, int numSquares, int offset) {
  byte bytes[CONFIG_BUFFER_SZ];

  // Write the overall configuration
  cube_config_t *config = (cube_config_t *)bytes;
  config->version = CUBE_VERSION;
  offset = EEPROM_safe_write(offset, bytes, sizeof (cube_config_t));

  // Write square info
  for (int face = 0; face < numSquares; face++) {
    int size = squares[face].toBytes(bytes, CONFIG_BUFFER_SZ);

    offset = EEPROM_safe_write(offset, bytes, size);
    if (offset < size) {
      DEBUG_ERR("Failed to write squares data");
      break;
    }
    DEBUG_VALUE(DEBUG_LOW, "Wrote face=", face);
    DEBUG_VALUELN(DEBUG_LOW, " offset=", offset);
  }
  DEBUG_VALUELN(DEBUG_LOW, "Wrote cube config. end address=", offset);

  return offset;
}
