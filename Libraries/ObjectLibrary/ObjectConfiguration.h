/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#ifndef OBJECT_CONFIGURATION_H
#define OBJECT_CONFIGURATION_H

#include "Arduino.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "RS485Utils.h"
#include "MPR121.h"

#define CONFIG_BUFFER_SZ 32 // Buffer size for reading and writing configs

int readHMTLConfiguration(config_hdr_t *config, 
                          output_hdr_t *outputs[],
                          config_max_t readoutputs[],
                          void *objects[],
                          byte max_outputs,
                          PixelUtil *pixels, 
                          RS485Socket *rs485,
                          MPR121 *mpr121);

#endif
