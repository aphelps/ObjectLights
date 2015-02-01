/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#ifndef SOUNDUNIT_H
#define SOUNDUNIT_H

#include "Arduino.h"
#include <ffft.h>

// Microphone connects to Analog Pin 0.  Corresponding ADC channel number
// varies among boards...it's ADC0 on Uno and Mega, ADC7 on Leonardo.
// Other boards may require different settings; refer to datasheet.
#ifdef __AVR_ATmega32U4__
 #define SOUND_PIN 7
#else
 #define SOUND_PIN 0
#endif

#define LIGHT_PIN  1
#define KNOB_PIN   4


#define NUM_COLUMNS 8

extern int16_t       capture[FFT_N];    // Audio capture buffer
extern uint16_t      spectrum[FFT_N/2]; // Spectrum output buffer

extern byte dotCount, // Frame counter for delaying dot-falling speed
  colCount; // Frame counter for storing past column data

extern uint16_t
  col[NUM_COLUMNS][10],   // Column levels for the prior 10 frames
  minLvlAvg[NUM_COLUMNS], // For dynamic adjustment of low & high ends of graph,
  maxLvlAvg[NUM_COLUMNS], // pseudo rolling averages for the prior few frames.
  colDiv[NUM_COLUMNS];    // Used when filtering FFT output to 8 columns
extern uint8_t colLeveled[NUM_COLUMNS]; // Column values adjusted for levels

extern uint16_t light_level;
extern uint16_t knob_level;

void sound_initialize();
boolean check_sound();
void setupFreeRun();

void cliHandler(char **tokens, byte numtokens);
extern byte verbosity;
extern uint16_t output_period;

#endif // SOUNDUNIT_H
