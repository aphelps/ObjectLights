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
  , MODE_CYCLE_PATTERN
  //  , MODE_CIRCLE_PATTERN
  //  , MODE_FADE_CYCLE
  //  , MODE_CAP_RESPONSE
  , MODE_STATIC_NOISE
  , MODE_SWITCH_RANDOM
  //  , MODE_LIGHT_CENTER
  //  , MODE_BAR_CIRCLE
  , MODE_CRAWL
  //  , MODE_ORBITS
};
#define VALID_MODES (sizeof (validModes) / sizeof (uint8_t))

square_mode_t modeFunctions[] = {
  squaresAllOn,
  NULL, //squaresTestPattern,
  NULL, //squaresSetupPattern,
  NULL, //squaresRandomNeighbor,
  squaresCyclePattern,
  NULL, //squaresCirclePattern,
  NULL, //squaresFadeCycle,
  NULL, //squaresCapResponse,
  squaresStaticNoise,
  squaresSwitchRandom,
  squaresLightCenter,
  NULL, //squaresBarCircle,
  squaresCrawl,
  squaresBlinkPattern,
  squaresOrbits
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
  250   // MODE_ORBITS
};

uint8_t current_mode = VALID_MODES - 1; // XXX
uint8_t previous_mode = 0;


/* Return the current mode value */
int get_current_mode(void)
{
  return validModes[current_mode % VALID_MODES];
}

boolean restorable = false;
void set_mode(uint8_t new_mode)
{
  if (current_mode != new_mode) {
    restorable = true;
    previous_mode = current_mode;
    current_mode = new_mode % VALID_MODES;
    DEBUG_VALUELN(DEBUG_HIGH, "Set mode=", current_mode);
  }
}

void increment_mode()
{
  set_mode((current_mode + 1) % VALID_MODES);
}

void restore_mode(void)
{
  if (restorable) {
    restorable = false;
    previous_mode = current_mode;
    current_mode = previous_mode;
    DEBUG_VALUELN(DEBUG_HIGH, "Restore mode=", current_mode);
  }
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
  MODE_LIGHT_CENTER,
  MODE_BLINK_PATTERN,
  (uint8_t)-1
};
#define VALID_FOLLOWUPS (sizeof (validFollowups) / sizeof (uint8_t))

uint8_t current_followup = (uint8_t)-1;
uint8_t previous_followup = (uint8_t)-1;


/* Return the current followup value */
int get_current_followup(void)
{
  if (current_followup == (uint8_t)-1) {
    return current_followup;
  }
  return validFollowups[current_followup % VALID_FOLLOWUPS];
}

boolean restorableFollowup = false;
void set_followup(uint8_t new_followup)
{
  if (current_followup != new_followup) {
    restorableFollowup = true;
    previous_followup = current_followup;
    current_followup = new_followup % VALID_FOLLOWUPS;
    DEBUG_VALUE(DEBUG_HIGH, "Set followup=", current_followup);
    DEBUG_VALUELN(DEBUG_HIGH, " prev=", previous_followup);
  }
}

void set_followup_mode(uint8_t mode) {
  for (byte i = 0; i < VALID_FOLLOWUPS; i++) {
    if (validFollowups[i] == mode) {
      set_followup(i);
      return;
    }
  }
}

void increment_followup()
{
  set_followup(current_followup + 1 );
}

void restore_followup(void)
{
  if (restorableFollowup) {
    uint8_t tmp;
    restorableFollowup = false;
    tmp = previous_followup;
    previous_followup = current_followup;
    current_followup = tmp;
    DEBUG_VALUELN(DEBUG_HIGH, "Restore followup=", current_followup);
  }
}
