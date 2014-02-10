/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#ifndef CUBE_CONFIGURATION_H
#define CUBE_CONFIGURATION_H

#include "SquareStructure.h"

typedef struct {
  byte version;
  byte reserved[7];
} cube_config_t;

int readConfiguration(Square *squares, int numSquares, int offset);
int writeConfiguration(Square *squares, int numSquares, int offset);

#endif
