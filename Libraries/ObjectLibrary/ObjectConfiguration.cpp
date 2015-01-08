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
#include <FastLED.h>
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

int readHMTLConfiguration(config_hdr_t *config, 
                          output_hdr_t *outputs[],
                          config_max_t readoutputs[],
                          byte max_outputs,
                          PixelUtil *pixels, 
                          RS485Socket *rs485,
                          MPR121 *mpr121) {
  int offset;

  uint32_t outputs_found = hmtl_setup(config, readoutputs, outputs,
                                      NULL, max_outputs,
                                      rs485, pixels, mpr121, 
                                      NULL, NULL, &offset);

  if ((rs485 != NULL) && !(outputs_found & (1 << HMTL_OUTPUT_RS485))) {
    DEBUG_ERR("No RS485 config found");
    DEBUG_ERR_STATE(1);
  }

  if ((pixels != NULL) && !(outputs_found & (1 << HMTL_OUTPUT_PIXELS))) {
    DEBUG_ERR("No pixels config found");
    DEBUG_ERR_STATE(2);
  }

  if ((mpr121 != NULL) && !(outputs_found & (1 << HMTL_OUTPUT_MPR121))) {
    DEBUG_ERR("No mpr121 config found");
    DEBUG_ERR_STATE(3);
  }

  return offset;
}
