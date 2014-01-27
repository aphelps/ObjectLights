/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 ******************************************************************************/

#include "CubeConfig.h"
#include "CubeConfiguration.h"
#include "SquareStructure.h"
#include "EEPromUtils.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#define READ_BUFFER_SZ 32
int readConfiguration(Square *squares, int numSquares, int offset) {
  byte bytes[READ_BUFFER_SZ];

  // Read the overall configuration
  offset = EEPROM_safe_read(offset, bytes, READ_BUFFER_SZ);
  cube_config_t *config = (cube_config_t *)bytes;
  DEBUG_VALUE(DEBUG_LOW, "Read version=", config->version);
  // XXX - What to do with config?

  // Read all squares and initialize them
  for (int face = 0; face < numSquares; face++) {
    offset = EEPROM_safe_read(offset, bytes, READ_BUFFER_SZ);
    if (offset <= 0) {
      DEBUG_ERR("Failed to read squares data");
      break;
    }
    squares[face].fromBytes(bytes, READ_BUFFER_SZ, squares, numSquares);

    DEBUG_VALUE(DEBUG_LOW, " - face=", face);
    DEBUG_VALUE(DEBUG_LOW, " offset=", offset);
    DEBUG_VALUE(DEBUG_LOW, " id=",  squares[face].id);
    DEBUG_VALUE(DEBUG_LOW, " top=", squares[face].edges[Square::TOP]->id);
  }
  DEBUG_PRINT_END();

  return offset;
}

int writeConfiguration(Square *squares, int numSquares, int offset) {
  byte bytes[32];

  // Write the overall configuration
  cube_config_t *config = (cube_config_t *)bytes;
  config->version = CUBE_VERSION;
  offset = EEPROM_safe_write(offset, bytes, sizeof (cube_config_t));

  // Write square info
  for (int face = 0; face < numSquares; face++) {
    int size = squares[face].toBytes(bytes, 32);

    offset = EEPROM_safe_write(offset, bytes, size);
    if (offset < size) {
      DEBUG_ERR("Failed to write squares data");
      break;
    }
    DEBUG_VALUE(DEBUG_LOW, "Wrote face=", face);
    DEBUG_VALUELN(DEBUG_LOW, " offset=", offset);
  }
  DEBUG_VALUELN(DEBUG_LOW, "Wrote config. end address=", offset);

  return offset;
}
