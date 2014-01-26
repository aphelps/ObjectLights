/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 ******************************************************************************/

#include "CubeConfiguration.h"
#include "SquareStructure.h"
#include "EEPromUtils.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

int readConfiguration(Square *squares, int numSquares, int offset) {
  byte bytes[32];
  for (int face = 0; face < numSquares; face++) {
    offset = EEPROM_safe_read(offset, bytes, 32);
    if (offset <= 0) {
      DEBUG_ERR("Failed to read squares data");
      break;
    }
    squares[face].fromBytes(bytes, 32);

    DEBUG_VALUE(DEBUG_LOW, "Read face=", face);
    DEBUG_VALUELN(DEBUG_LOW, " offset=", offset);
  }

  return offset;
}

int writeConfiguration(Square *squares, int numSquares, int offset) {
  byte bytes[32];
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
