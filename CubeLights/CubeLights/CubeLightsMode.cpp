/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/
#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include <Arduino.h>

#include "CubeLights.h"

unsigned long mode_next_action = 0;

/* 
 * Array of modes that are valid for selection.  The value are indices
 * into the modeFunctions array.
 */
uint8_t validModes[] = {
  MODE_ALL_ON
  //   , MODE_TEST_PATTERN
  //   , MODE_SETUP_PATTERN
  //   , MODE_RANDOM_NEIGHBOR
  //  , MODE_CYCLE_PATTERN
  //  , MODE_CIRCLE_PATTERN
  //  , MODE_FADE_CYCLE
  //  , MODE_CAP_RESPONSE
  , MODE_STATIC_NOISE
  , MODE_SWITCH_RANDOM
  //  , MODE_LIGHT_CENTER
  //  , MODE_BAR_CIRCLE
  //  , MODE_CRAWL
  , MODE_BLINK_PATTERN
  // , MODE_ORBIT_TEST
  , MODE_VECTORS
  , MODE_SIMPLE_LIFE
  //, MODE_SOUND_TEST
  , MODE_SOUND_HMTL
  //, MODE_STROBE
};
#define VALID_MODES (sizeof (validModes) / sizeof (uint8_t))

square_mode_t modeFunctions[] = {
  squaresAllOn,
  NULL, //squaresTestPattern,
  NULL, //squaresSetupPattern,
  NULL, //squaresRandomNeighbor,
  NULL, //squaresCyclePattern,
  NULL, //squaresCirclePattern,
  NULL, //squaresFadeCycle,
  NULL, //squaresCapResponse,
  squaresStaticNoise,
  squaresSwitchRandom,
  NULL, //squaresLightCenter,
  NULL, //squaresBarCircle,
  NULL, //squaresCrawl,
  squaresBlinkPattern,
  NULL, //squaresOrbitTest,
  squaresVectors,
  squaresSimpleLife,
  NULL, //squaresSoundTest,
  squaresSoundHMTL,
  NULL, //squaresStrobe
};
#define NUM_MODES (sizeof (modeFunctions) / sizeof (square_mode_t))

/* Update period (in ms) for each mode */
uint16_t modePeriods[] = {
  1,    // MODE_ALL_ON
  1000, // MODE_TEST_PATTERN
  1000, // MODE_SETUP_PATTERN
  500,  // MODE_RANDOM_NEIGHBOR
  500,  // MODE_CYCLE_PATTERN
  500,  // MODE_CIRCLE_PATTERN
  1,    // MODE_FADE_CYCLE
  500,  // MODE_CAP_RESPONSE
  250,  // MODE_STATIC_NOISE
  100,  // MODE_SWITCH_RANDOM
  1,    // MODE_LIGHT_CENTER
  500,  // MODE_BAR_CIRCLE
  100,  // MODE_CRAWL
  250,  // MODE_BLINK_PATTERN
  250,  // MODE_ORBITS
  100,  // MODE_VECTORS
  500,  // MODE_SIMPLE_LIFE
  100,  // MODE_SOUND_TEST
  100,  // MODE_SOUND_TEST2
  1     // MODE_STROBE
};

#ifndef CUBE_START_MODE
#define CUBE_START_MODE 0 // VALID_MODES - 2
#else
#if CUBE_START_MODE > VALID_MODES
#error Invalid start mode
#endif
#endif

uint8_t current_modes[MAX_MODES] = {
  CUBE_START_MODE,  // This is the starting mode
  MODE_NONE,
  MODE_NONE
};
uint8_t previous_modes[MAX_MODES] = {
  MODE_NONE,
  MODE_NONE,
  MODE_NONE
};

/* Return the current mode value */
uint8_t get_current_mode(uint8_t place)
{
  if (current_modes[place] == MODE_NONE) {
    return MODE_NONE;
  }
  return validModes[current_modes[place] % VALID_MODES];
}

void set_mode(uint8_t place, uint8_t new_mode) {
  if (current_modes[place] != new_mode) {
    previous_modes[place] = current_modes[place];
    current_modes[place] = new_mode;

    // Set next_time to zero to trigger initialization
    modeConfigs[place].next_time = 0;

    DEBUG3_VALUE("Set mode ", place);
    DEBUG3_VALUELN("=", current_modes[place]);

    // Reset the brightness
    FastLED.setBrightness(255);
  }
}

void set_mode_to(uint8_t place, uint8_t mode) {
  if (mode == MODE_NONE) {
    set_mode(place, MODE_NONE);
    return;
  }

  for (byte i = 0; i < VALID_MODES; i++) {
    if (validModes[i] == mode) {
      set_mode(place, i);
      return;
    }
  }
  DEBUG1_VALUELN("Attempted to set invalid mode:", mode);
}

void increment_mode(uint8_t place)
{
  set_mode(place, (current_modes[place] + 1) % VALID_MODES);
}

/*
 * Restore the previous mode (if there is one) and clear the previous mode
 */
void restore_mode(uint8_t place)
{
  set_mode(place, previous_modes[place]);
  previous_modes[place] = MODE_NONE;
  DEBUG4_VALUE("Restored mode ", place);
  DEBUG4_VALUELN("=", current_modes[place]);
}

/***** Followups *************************************************************/

// XXX - This is currently a straight copy, need to refactor and combine


uint8_t validFollowups[] = {
  //  MODE_ALL_ON
  //   , MODE_TEST_PATTERN
  //   , MODE_SETUP_PATTERN
  //  , MODE_RANDOM_NEIGHBOR
  //  , MODE_CYCLE_PATTERN
  // , MODE_CIRCLE_PATTERN
  //  , MODE_FADE_CYCLE
  //  , MODE_CAP_RESPONSE
  //  , MODE_STATIC_NOISE
  //  , MODE_SWITCH_RANDOM
  //MODE_LIGHT_CENTER,
  MODE_BLINK_PATTERN,
  (uint8_t)-1
};
#define VALID_FOLLOWUPS (sizeof (validFollowups) / sizeof (uint8_t))

// XXX - Followups vs normal modes should be distiguished again, somehow???
