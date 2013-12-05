#define DEBUG_LEVEL DEBUG_MID
#include <Debug.h>

#include <Arduino.h>

#include "CubeLights.h"

unsigned long mode_next_action = 0;

/* Array of modes that are valid for normal use */
uint8_t validModes[] = {
//  MODE_SENSE_DISTANCE,
//  MODE_RANDOM_FADES,
  MODE_SWAP_ONE,
//  MODE_FADE_ONE,
  MODE_SET_ALL,
//  MODE_FLASH_ORDERED,
};
uint8_t current_mode = validModes[0];
uint8_t previous_mode = 0;


/* Return the current mode value */
#define MODE_CHANGE_PERIOD (5 * 60 * 1000) // Period between mode changes
int get_current_mode(void)
{
#if 0
    static unsigned long lastChange = millis();
  if (lastChange < (millis() - MODE_CHANGE_PERIOD)) {
    mode = (mode + 1) % MODE_TOTAL;
  }
#endif
  
  if (current_mode != previous_mode) {
    DEBUG_VALUE(2, F(" New mode: "), current_mode);
    previous_mode = current_mode;

    // XXX    Tlc.setAll(0);
  }

  return current_mode;
}

boolean restorable = false;
void set_current_mode(uint8_t new_mode) 
{
  if (current_mode != new_mode) {
    restorable = true;
    current_mode = new_mode;
  }
}

void restore_current_mode(void) 
{
  if (restorable) {
    restorable = false;
    current_mode = previous_mode;
  }
}

boolean update_needed = false;
void send_update() {
  if (update_needed) {
    ///XXX    while (Tlc.update());
    update_needed = true;
  }
}
