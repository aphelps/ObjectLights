#include "Arduino.h"

#include "Tlc5940.h"
#include "tlc_shifts.h"
#include "tlc_fades.h"

#include "UmbrellaSign.h"

int mode = MODE_SWAP_ONE; // Starting mode

/* Return the current mode value */
#define MODE_CHANGE_PERIOD (5 * 60 * 1000) // Period between mode changes
int get_current_mode(void) 
{
  static unsigned long lastChange = millis();

  if (lastChange < (millis() - MODE_CHANGE_PERIOD)) {
    mode = (mode + 1) % MODE_TOTAL;
  }

  return mode;
}

/* From TCL5940 Library's CircularLightBuffer example */
int mode_example_circular(void *arg) 
{
  uint16_t sum = tlc_shiftUp() + 256 * 4;
  if (sum > MAX_VALUE)
    sum = 0;
  Tlc.set(0, sum);

  while (Tlc.update());

  return (2000 / 16);
}

/* From TCL5940 Library's Fades example */
int mode_example_fades(void *arg)
{
  static TLC_CHANNEL_TYPE channel;
  if (tlc_fadeBufferSize < TLC_FADE_BUFFER_LENGTH - 2) {
    if (!tlc_isFading(channel)) {
      uint16_t duration = analogRead(0) * 2;
      int maxValue = analogRead(0) * 2;
      uint32_t startMillis = millis() + 50;
      uint32_t endMillis = startMillis + duration;
      tlc_addFade(channel, 0, maxValue, startMillis, endMillis);
      tlc_addFade(channel, maxValue, 0, endMillis, endMillis + duration);
    }
    if (channel++ == NUM_TLCS * 16) {
      channel = 0;
    }
  }
  tlc_updateFades();

  return 0;
}

/* Set all LEDs to their max value */
int mode_all_on(void *arg) 
{
  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues[led] = MAX_VALUE;
    Tlc.set(led, ledValues[led]);
  }

  while (Tlc.update());

  return 1000;
}

/* Swap the state of a single LED */
int mode_swap_one(void *arg) 
{
  int led = random(NUM_LEDS);

  if (ledValues[led]) ledValues[led] = 0;
  else ledValues[led] = MAX_VALUE;

  Tlc.set(led, ledValues[led]);

  while (Tlc.update());

  return 100;
}

/* Fade the state of a single LED from high-to-low or vice versa*/
#define FADE_ONE_MIN_DURATION 500
#define FADE_ONE_MAX_DURATION 4000
int mode_fade_one(void *arg)
{
  /* Get a random LED and determine the start and end values for the fade */
  int led = random(NUM_LEDS);
  uint32_t startValue = ledValues[led];
  uint32_t endValue;
  if (startValue) endValue = 0;
  else endValue = MAX_VALUE;

  /* Set duration of the fade */
  uint32_t duration = FADE_ONE_MIN_DURATION +
    random(FADE_ONE_MAX_DURATION - FADE_ONE_MIN_DURATION);

  /* Set the start time slightly in the future */
  uint32_t startMillis = millis() + 50;

  tlc_addFade(led, startValue, endValue, startMillis, startMillis + duration);

  while (tlc_updateFades()); /* Updae until the fade completes */

  ledValues[led] = endValue;

  return 0;
}

/*
 * This mode fades the rows of the sign one at a time
 */
#define FADE_ROW_DURATION 2000
int mode_fade_row(void *arg) 
{
  static uint8_t row = 0;

  /* Fade to the opposite value */
  uint32_t startValue = rowValues[row];
  uint32_t endValue;
  if (startValue) endValue = 0;
  else endValue = MAX_VALUE;

  uint32_t startMillis = millis() + 50;
  uint32_t endMillis = startMillis + FADE_ROW_DURATION;

  /* Start a fade for each led of the row */
  for (int led = 0; led < NUM_LEDS; led++) {
    if (ledRow[led] == row) {
      tlc_addFade(led, startValue, endValue,
                  startMillis, endMillis);
      ledValues[led] = endValue;
    }
  }

  while (tlc_updateFades()); /* Update until the fade completes */

  /* Store the new row value */
  rowValues[row] = endValue;

  /* Move to the next row */
  row = (row + 1) % MAX_ROW;

  return 0;
}
