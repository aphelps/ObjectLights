/*
 * This code drive's Adam's Umbrella sign.
 *
 * Internal to the sign is an Arduino Nano v3 driving two serially connected
 * TLC5940 LED drivers for the individual pixels of the sign.
 */

#include "Arduino.h"

#include "Tlc5940.h"

#include "UmbrellaSign.h"

#if NUM_TLCS != 2
  /* NUM_TLCS must be set to 2 in tlc_config.h */
  NUM_TLCS must equal 2;
#endif

/* Array to map the light positions in the sign to the LED driver pins */
#define XX 0 // XXX: Remove when actual mapping is determined
uint8_t signTLCPin[] =
{
  XX, XX, XX, XX, XX, -1, -1, -1,
  -1, -1, XX, XX, XX, XX, XX, -1,
  -1, -1, -1, XX, XX, XX, XX, -1,
  -1, -1, -1, -1, XX, XX, XX, XX,
  -1, -1, -1, -1, -1, XX, XX, XX,
  -1, -1, XX, -1, -1, -1, XX, XX
  -1, XX, -1, -1, -1, -1, -1, XX,
  XX, -1, -1, -1, -1, -1, -1, XX,
};

/* Mapping of LEDs into rows along the diaginal axis */
uint8_t signRows[] =
{
   5,  4,  3,  2,  1, -1, -1, -1,
  -1, -1,  4,  3,  2,  1,  0, -1,
  -1, -1, -1,  4,  3,  2,  1, -1,
  -1, -1, -1, -1,  4,  3,  2,  1,
  -1, -1, -1, -1, -1,  4,  3,  2,
  -1, -1,  6, -1, -1, -1,  4,  3
  -1,  7, -1, -1, -1, -1, -1,  4,
   8, -1, -1, -1, -1, -1, -1,  5,
};

/* TCL Pin values */
uint16_t ledValues[NUM_LEDS] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
};

mode_function_t modeFunctions[] = {
  mode_example_circular, // MODE_EXAMPLE_CIRCULAR
  mode_example_fades,    // MODE_EXAMPLE_FADES
  mode_all_on,           // MODE_ALL_ON
  mode_swap_one,         // MODE_SWAP_ONE
  mode_fade_one          // MODE_FADE_ONE
};

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  /* Initialize the LED drivers with all-on */ 
  Tlc.init(MAX_VALUE);

  /* Initialize the LED values */
  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues[led] = MAX_VALUE;
  }

  randomSeed(analogRead(0));
}

/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
  /* Get the current mode */
  int mode = get_current_mode();

  /* Call the action function for the current mode */
  int delay_period = modeFunctions[mode](NULL);

  /* Wait for the specifided interval */
  delay(delay_period);
}

