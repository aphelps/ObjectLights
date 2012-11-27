/*
 * This code drive's Adam's Umbrella sign.
 *
 * Internal to the sign is an Arduino Nano v3 driving two serially connected
 * TLC5940 LED drivers for the individual pixels of the sign.
 */

#include <Arduino.h>

#include "Tlc5940.h"

#include "UmbrellaSign.h"

#if NUM_TLCS != 2
  /* NUM_TLCS must be set to 2 in tlc_config.h */
  NUM_TLCS must equal 2;
#endif

/* Array to map the light positions in the sign to the LED driver pins */
#define XX 0 // XXX: Remove when actual mapping is determined
int8_t signToIndex[] =
{
  19, 10, 8,  18, 12, -1, -1, -1,
  -1, -1, 11, 16,  7,  9, 17, -1,
  -1, -1, -1, 20, 22, 21,  4, -1,
  -1, -1, -1, -1, 23,  5, 13,  3,
  -1, -1, -1, -1, -1, 25,  6,  1,
  -1, -1, 26, -1, -1, -1, 15, 24,
  -1,  0, -1, -1, -1, -1, -1, 14,
  27, -1, -1, -1, -1, -1, -1,  2,
};

/* Mapping of LEDs into rows along the diaginal axis */
int8_t signRows[] =
{
   5,  4,  3,  2,  1, -1, -1, -1,
  -1, -1,  4,  3,  2,  1,  0, -1,
  -1, -1, -1,  4,  3,  2,  1, -1,
  -1, -1, -1, -1,  4,  3,  2,  1,
  -1, -1, -1, -1, -1,  4,  3,  2,
  -1, -1,  6, -1, -1, -1,  4,  3,
  -1,  7, -1, -1, -1, -1, -1,  4,
   8, -1, -1, -1, -1, -1, -1,  5,
};

int8_t ledRow[NUM_LEDS];
uint16_t rowValues[MAX_ROW] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE,
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
  mode_fade_one,         // MODE_FADE_ONE
  mode_fade_row,         // MODE_FADE_ROW
  mode_count_up,         // MODE_COUNT_UP
  mode_flash_ordered,    // MODE_FLASH_ORDERED
};


/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  Serial.begin(9600);

  /* Initialize the LED drivers with all-on */ 
  Tlc.init(MAX_VALUE);

  /* Initialize the LED values */
  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues[led] = MAX_VALUE;
  }

  randomSeed(analogRead(0));

  /* Populate the ledRowArray */
  DEBUG_PRINT("ledRowArray:\n");
  for (uint32_t i = 0; i < sizeof (signRows); i++) {
    int8_t led = signToIndex[i];
    if (led < 0) {
      DEBUG_PRINT("X/X");
    } else {
      ledRow[led] = signRows[i];
      DEBUG_PRINT(led);
      DEBUG_PRINT("/");
      DEBUG_PRINT(ledRow[led]);
    }
    if (i % 8 == 7) {
      DEBUG_PRINT("\n");
    } else {
      DEBUG_PRINT(", ");
    }
  }

  DEBUG_PRINT("ledRow:");
  for (int i = 0; i < NUM_LEDS; i++) {
    DEBUG_PRINT(ledRow[i]);
    DEBUG_PRINT(", ");
  }
  DEBUG_PRINT("\n");

  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(0, buttonInterrupt, CHANGE);
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

