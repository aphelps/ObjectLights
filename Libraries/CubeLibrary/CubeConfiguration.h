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

#define CONFIG_BUFFER_SZ 32 // Buffer size for reading and writing configs

void readHMTLConfiguration(config_hdr_t *config, 
                           output_hdr_t *outputs[],
                           config_max_t readoutputs[],
                           byte max_outputs);
int readCubeConfiguration(Square *squares, int numSquares, int offset);
int writeCubeConfiguration(Square *squares, int numSquares, int offset);

#endif
