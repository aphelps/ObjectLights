/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#ifndef OBJECT_CONFIGURATION_H
#define OBJECT_CONFIGURATION_H

#define CONFIG_BUFFER_SZ 32 // Buffer size for reading and writing configs

int readHMTLConfiguration(PixelUtil *pixels, RS485Socket *rs485, MPR121 *mpr121);

#endif
