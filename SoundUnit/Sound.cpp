/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Derived from: https://github.com/adafruit/piccolo
 ******************************************************************************/

#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include <avr/pgmspace.h>
#include <ffft.h>
#include <math.h>

#include "SoundUnit.h"

uint16_t light_level;
uint16_t knob_level;

int16_t       capture[FFT_N];    // Audio capture buffer
volatile byte samplePos = 0;     // Buffer position counter

complex_t     bfly_buff[FFT_N];  // FFT "butterfly" buffer
uint16_t      spectrum[FFT_N/2]; // Spectrum output buffer
 
byte
  dotCount = 0, // Frame counter for delaying dot-falling speed
  colCount = 0; // Frame counter for storing past column data
uint16_t
  col[NUM_COLUMNS][10],   // Column levels for the prior 10 frames
  minLvlAvg[NUM_COLUMNS], // For dynamic adjustment of low & high ends of graph,
  maxLvlAvg[NUM_COLUMNS], // pseudo rolling averages for the prior few frames.
  colDiv[NUM_COLUMNS];    // Used when filtering FFT output to 8 columns
uint8_t colLeveled[NUM_COLUMNS]; // Column values adjusted for levels

// TODO: colDiv appears to just be the summation of values from progmem, could
// be converted to progrmen

/*
  These tables were arrived at through testing, modeling and trial and error,
  exposing the unit to assorted music and sounds.  But there's no One Perfect
  EQ Setting to Rule Them All, and the graph may respond better to some
  inputs than others.  The software works at making the graph interesting,
  but some columns will always be less lively than others, especially
  comparing live speech against ambient music of varying genres.
*/
PROGMEM uint8_t
// This is low-level noise that's subtracted from each FFT output column:
  noise[64]={ 8,6,6,5,3,4,4,4,3,4,4,3,2,3,3,4,
              2,1,2,1,3,2,3,2,1,2,3,1,2,3,4,4,
              3,2,2,2,2,2,2,1,3,2,2,2,2,2,2,2,
              2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,4 },
  // These are scaling quotients for each FFT output column, sort of a
  // graphic EQ in reverse.  Most music is pretty heavy at the bass end.
  eq[64]={
    255, 175,218,225,220,198,147, 99, 68, 47, 33, 22, 14,  8,  4,  2,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },

  // When filtering down to 8 columns, these tables contain indexes
  // and weightings of the FFT spectrum output values to use.  Not all
  // buckets are used -- the bottom-most and several at the top are
  // either noisy or out of range or generally not good for a graph.
  col0data[] = {  2,  1,  // # of spectrum bins to merge, index of first
    111,   8 },           // Weights for each bin
  col1data[] = {  4,  1,  // 4 bins, starting at index 1
     19, 186,  38,   2 }, // Weights for 4 bins.  Got it now?
  col2data[] = {  5,  2,
     11, 156, 118,  16,   1 },
  col3data[] = {  8,  3,
      5,  55, 165, 164,  71,  18,   4,   1 },
  col4data[] = { 11,  5,
      3,  24,  89, 169, 178, 118,  54,  20,   6,   2,   1 },
  col5data[] = { 17,  7,
      2,   9,  29,  70, 125, 172, 185, 162, 118, 74,
     41,  21,  10,   5,   2,   1,   1 },
  col6data[] = { 25, 11,
      1,   4,  11,  25,  49,  83, 121, 156, 180, 185,
    174, 149, 118,  87,  60,  40,  25,  16,  10,   6,
      4,   2,   1,   1,   1 },
  col7data[] = { 37, 16,
      1,   2,   5,  10,  18,  30,  46,  67,  92, 118,
    143, 164, 179, 185, 184, 174, 158, 139, 118,  97,
     77,  60,  45,  34,  25,  18,  13,   9,   7,   5,
      3,   2,   2,   1,   1,   1,   1 },
  // And then this points to the start of the data for each of the columns:
  *colData[] = {
    col0data, col1data, col2data, col3data,
    col4data, col5data, col6data, col7data };


/*
 * Sound sensing initialization
 */
void sound_initialize() {
  uint8_t i, j, nBins, *data;

  memset(col , 0, sizeof(col));

  for (i = 0; i< NUM_COLUMNS; i++) {
    minLvlAvg[i] = 0;
    maxLvlAvg[i] = 512;

    data         = (uint8_t *)pgm_read_word(&colData[i]);
    nBins        = pgm_read_byte(&data[0]);
    for(colDiv[i]=0, j=0; j<nBins; j++)
      colDiv[i] += pgm_read_byte(&data[j + 2]);
  }

  setupFreeRun();
}

volatile byte current_pin = SOUND_PIN;
volatile int ready_pin = -1;

/*
 * Setup ADC free-run mode
 */
void setupFreeRun() {
  // Init ADC free-run mode; f = ( 16MHz/prescaler ) / 13 cycles/conversion
  ADMUX  = current_pin; // Channel sel, right-adj, use AREF pin
  ADCSRA = _BV(ADEN)  | // ADC enable
           _BV(ADSC)  | // ADC start
           _BV(ADATE) | // Auto trigger
           _BV(ADIE)  | // Interrupt enable
           _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); // 128:1 / 13 = 9615 Hz
  ADCSRB = 0;                // Free run mode, no high MUX bit
  DIDR0  = 1 << SOUND_PIN // Turn off digital input for ADC pin
         | 1 << LIGHT_PIN 
         | 1 << KNOB_PIN;
  TIMSK0 = 0;                // Timer0 off

  sei(); // Enable interrupts
}

/*
 * Sampling interupt
 *   - Cycles through the sensor pins, taking FFT_N audio samples and a single
 *     sample of all others.
 */
ISR(ADC_vect) {
  int16_t sample = ADC; // 0-1023
  boolean done = false;

  if (current_pin == SOUND_PIN) {
    static const int16_t noiseThreshold = 4;

    // XXX: Why ignore values between 508-516?
    capture[samplePos] =
      ((sample > (512 - noiseThreshold)) &&
       (sample < (512 + noiseThreshold))) ? 0 :
      sample - 512; // Sign-convert for FFT; -512 to +511

    if (++samplePos >= FFT_N) done = true;
  } else if (current_pin == LIGHT_PIN) {
    light_level = sample; // Does there need to be input pullup or pull down resistor?
    done = true;
  } else if (current_pin == KNOB_PIN) {
    knob_level = sample;
    done = true;
  }

  if (done) {
    // Record which pin is done sampling
    ready_pin = current_pin;

    // Turn off interrupts to report back and switch pins
    ADCSRA &= ~_BV(ADIE);
    
    // Switch pins
    switch (current_pin) {
      case SOUND_PIN: current_pin = KNOB_PIN; break;
      case KNOB_PIN: current_pin = LIGHT_PIN; break;
      case LIGHT_PIN: current_pin = SOUND_PIN; break;
    }
    ADMUX  = current_pin;
  }
}

void processSound() {

  uint8_t  i, x, L, *data, nBins, binNum;
  uint16_t minLvl, maxLvl;
  int      level, sum;

  fft_input(capture, bfly_buff);   // Samples -> complex #s
  samplePos = 0;                   // Reset sample counter
  ADCSRA |= _BV(ADIE);             // Resume sampling interrupt
  fft_execute(bfly_buff);          // Process complex data
  fft_output(bfly_buff, spectrum); // Complex -> spectrum

  // Remove noise and apply EQ levels
  for (x=0; x < FFT_N / 2; x++) {
    L = pgm_read_byte(&noise[x]);
    spectrum[x] = (spectrum[x] <= L) ? 0 :
      (((spectrum[x] - L) * (256L - pgm_read_byte(&eq[x]))) >> 8);
  }

  colCount = (colCount + 1) % 10;

  // Downsample spectrum output to 8 columns:
  for(x = 0; x < NUM_COLUMNS; x++) {
    data   = (uint8_t *)pgm_read_word(&colData[x]);
    nBins  = pgm_read_byte(&data[0]);
    binNum = pgm_read_byte(&data[1]);
    for(sum = 0, i = 0; i < nBins; i++)
      sum += spectrum[binNum++] * pgm_read_byte(&data[i + 2]); // Weighted
    col[x][colCount] = sum / colDiv[x];                    // Average
    minLvl = maxLvl = col[x][0];
    for(i = 1; i < 10; i++) { // Get range of prior 10 frames
      if(col[x][i] < minLvl)      minLvl = col[x][i];
      else if(col[x][i] > maxLvl) maxLvl = col[x][i];
    }
    // minLvl and maxLvl indicate the extents of the FFT output, used
    // for vertically scaling the output graph (so it looks interesting
    // regardless of volume level).  If they're too close together though
    // (e.g. at very low volume levels) the graph becomes super coarse
    // and 'jumpy'...so keep some minimum distance between them (this
    // also lets the graph go to zero when no sound is playing):
    if((maxLvl - minLvl) < 8) maxLvl = minLvl + 8;
    minLvlAvg[x] = (minLvlAvg[x] * 7 + minLvl) >> 3; // Dampen min/max levels
    maxLvlAvg[x] = (maxLvlAvg[x] * 7 + maxLvl) >> 3; // (fake rolling average)

    // Second fixed-point scale based on dynamic min/max levels:
    level = 10L * (col[x][colCount] - minLvlAvg[x]) /
      (long)(maxLvlAvg[x] - minLvlAvg[x]);

    // Clip output and convert to byte:
    if(level < 0L)      colLeveled[x] = 0;
    else if(level > 10) colLeveled[x] = 10; // Allow dot to go a couple pixels off top
    else                colLeveled[x] = (uint8_t)level;

    // XXX - The leveled columns could probably be improved
  }
}


boolean check_sound() {
  if (!(ADCSRA & _BV(ADIE))) {
    // Current sample is complete
    if (ready_pin == SOUND_PIN) {
      processSound(); // Sampling is re-enabled in processSound()
      return true;
    } else {
      ADCSRA |= _BV(ADIE); // Resume sampling interrupt
    }
  }

  return false;
}
