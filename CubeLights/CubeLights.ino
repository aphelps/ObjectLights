/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/


#define DEBUG_LEVEL 2
#include <Debug.h>

#include <Arduino.h>
#include <CapacitiveSensor.h>
#include <NewPing.h>
#include "Tlc5940.h"

#include "CubeLights.h"

#if NUM_TLCS != 1
  /* NUM_TLCS must be set to 1 in tlc_config.h */
  NUM_TLCS must equal 1;
#endif

/* TCL Pin values */
int16_t ledValues[NUM_LEDS] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
};

mode_function_t modeFunctions[] = {
  mode_set_all,          // MODE_SET_ALL
  mode_swap_one,         // MODE_SWAP_ONE
  mode_fade_one,         // MODE_FADE_ONE
  mode_count_up,         // MODE_COUNT_UP
  mode_flash_ordered,    // MODE_FLASH_ORDERED
  mode_random_fades,     // MODE_RANDOM_FADES
  mode_sense_distance,   // MODE_SENSE_DISTANCE
};

void * modeArguments[] = {
  (void *)MAX_VALUE,    // MODE_SET_ALL
  NULL,                 // MODE_SWAP_ONE
  NULL,                 // MODE_FADE_ONE
  NULL,                 // MODE_COUNT_UP
  NULL,                 // MODE_FLASH_ORDERED
  NULL,                 // MODE_RANDOM_FADES
  NULL,                 // MODE_SENSE_DISTANCE
};

#define INITIAL_VALUE 0

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  //Serial.begin(9600);
  Serial.begin(115200);

  sensor_cap_init(); /* Initialize the capacitive sensors */

  /* Initialize the LED drivers with all-off */ 
  Tlc.init(INITIAL_VALUE);

  /* Initialize the LED values */
  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues[led] = INITIAL_VALUE;
  }

  randomSeed(analogRead(0));

  /* Initialize the range sensor */
//  pinMode(PING_TRIG, OUTPUT);
//  pinMode(PING_ECHO,INPUT);

  /* Turn on input pullup on analog photo pin */
  digitalWrite(PHOTO_PIN, HIGH); 
}



/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
  /* Update sensors as needed */
  sensor_cap();
  sensor_range();
  sensor_photo();

  int delay_period_ms;
  if (photo_dark) {
    /* Get the current mode */
    int mode = get_current_mode();

    /* Call the action function for the current mode */
    delay_period_ms = modeFunctions[mode](modeArguments[mode]);

    DEBUG_VALUE(3, "Mode:", mode);
  } else {
    /* When its light out then turn the lights off */
    delay_period_ms = mode_set_all(0);
  }

  DEBUG_VALUELN(3, "Per:", delay_period_ms);
  
  /* Wait for the specifided interval */
  delay(delay_period_ms);
}
