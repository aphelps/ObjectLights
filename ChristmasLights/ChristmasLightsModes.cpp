#include <Arduino.h>

#include "Tlc5940.h"
#include "tlc_shifts.h"
#include "tlc_fades.h"

#include "UmbrellaSign.h"

/* Array of modes that are valid for normal use */
uint8_t validModes[] = {
//  MODE_CROSS_FADE,
  MODE_RANDOM_FADES,
  MODE_SWAP_ONE,
//  MODE_FADE_COLUMN,  
  MODE_FADE_ONE,
  MODE_FADE_ROW,  
  MODE_ALL_ON,
  MODE_FLASH_ORDERED,
};

volatile uint16_t buttonValue = 0;
void buttonInterrupt(void) 
{
  static unsigned long prevTime = 0;
  static int prevValue = LOW;
  int value = digitalRead(PUSH_BUTTON_PIN);

  /* Provide a debounce to only change on the first interrupt */
  if ((value == HIGH) && (prevValue == LOW) && (millis() - prevTime > 500)) {
    buttonValue++;
    prevTime = millis();
    
    DEBUG_PRINT(value);
    DEBUG_PRINT("->");
    DEBUG_PRINT(buttonValue);
    DEBUG_PRINT("->");
    DEBUG_PRINT(prevTime);
    DEBUG_PRINT("\n");
  }

  prevValue = value;
}

/* Return the current mode value */
#define MODE_CHANGE_PERIOD (5 * 60 * 1000) // Period between mode changes
int get_current_mode(void) 
{
  static int prevMode = 0;
#if 0
  static unsigned long lastChange = millis();
  if (lastChange < (millis() - MODE_CHANGE_PERIOD)) {
    mode = (mode + 1) % MODE_TOTAL;
  }
#endif

  /* Increment the mode on button push */
  int mode = validModes[buttonValue % sizeof(validModes)];
  if (mode != prevMode) {
    DEBUG_PRINT("New mode: ");
    DEBUG_PRINT(mode);
    DEBUG_PRINT("\n");
    prevMode = mode;

    Tlc.setAll(0);
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
  Tlc.setAll(MAX_VALUE);

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

  return 25;
}

/* Fade the state of a single LED from high-to-low or vice versa*/
#define FADE_ONE_MIN_DURATION 50
#define FADE_ONE_MAX_DURATION 250
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
#define FADE_ROW_DURATION 500
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

/*
 * This mode fades the columns of the sign one at a time
 */
#define FADE_COLUMN_DURATION 500
int mode_fade_column(void *arg) 
{
  static uint8_t column = 0;

  /* Fade to the opposite value */
  uint32_t startValue = columnValues[column];
  uint32_t endValue;
  if (startValue) endValue = 0;
  else endValue = MAX_VALUE;

  uint32_t startMillis = millis() + 50;
  uint32_t endMillis = startMillis + FADE_COLUMN_DURATION;

  /* Start a fade for each led of the column */
  for (int led = 0; led < NUM_LEDS; led++) {
    if (ledColumn[led] == column) {
      tlc_addFade(led, startValue, endValue,
                  startMillis, endMillis);
      ledValues[led] = endValue;
    }
  }

  while (tlc_updateFades()); /* Update until the fade completes */

  /* Store the new column value */
  columnValues[column] = endValue;

  /* Move to the next column */
  column = (column + 1) % MAX_COLUMN;

  return 0;
}


/* Set and updated a single LED value */
void updateChannel(int channel, int value) 
{
  Tlc.set(channel, value);
  while(Tlc.update());
}

/* Flash a single LED */
int flashChannel(int channel, int count, int flash_delay)
{
  for (int i = 0; i < count; i++) {
    updateChannel(channel, MAX_VALUE);
    delay(flash_delay);

    updateChannel(channel, 0);
    delay(flash_delay);
  }

  return count * flash_delay;
}

/* Sequencially flash the LEDs */
int mode_count_up(void *arg) 
{
  for (int i = 0; i < NUM_LEDS; i++) {
    updateChannel(i, 0);
    delay(5000);
    flashChannel(i, i + 1, 400);
    delay(1000);
    flashChannel(i, i + 1, 400);
    updateChannel(i, 1);
    delay(1000);
  }
  return 0;
}


/* Blink the LEDs in left-right/top-down order */
uint8_t orderedLeds[] = {
  19, 10, 8, 18, 12,
         11, 16,  7,  9, 17,
             20, 22, 21,  4,
                 23,  5, 13,  3,
                     25,  6,  1,
         26,             15, 24,
       0,                    14,
  27,                         2,
};
int mode_flash_ordered(void *arg) 
{
  static int led = -1;
  led = (led + 1) % NUM_LEDS;
  if (ledValues[orderedLeds[led]]) {
    ledValues[orderedLeds[led]] = 0;
  } else {
    ledValues[orderedLeds[led]] = MAX_VALUE;
  }
  updateChannel(orderedLeds[led], ledValues[orderedLeds[led]]);
  return 250;
}

#define FADE_INCREMENT (1 << 6)
#define INCREASING_BIT (1 << 2)
int mode_cross_fade(void *arg) 
{
  static uint8_t column = 0;
  static uint8_t row = 0;
  
  for (int led = 0; led < NUM_LEDS; led++) {
    if (ledColumn[led] == column) {
      if (ledValues[led] <= 0) {
        ledValues[led] = FADE_INCREMENT | INCREASING_BIT;
      } else if (ledValues[led] >= MAX_VALUE) {
        ledValues[led] -= FADE_INCREMENT | INCREASING_BIT;
      } else if (ledValues[led] & INCREASING_BIT) {
        ledValues[led] += FADE_INCREMENT;
        if (ledValues[led] > MAX_VALUE) {
          ledValues[led] = MAX_VALUE;
        }
      } else {
        ledValues[led] -= FADE_INCREMENT;
      }
      DEBUG_PRINT(led);
      DEBUG_PRINT("c");
      DEBUG_PRINT(ledValues[led]);
      DEBUG_PRINT("-");
    }
    if (ledRow[led] == row) {
      if (ledValues[led] <= 0) {
        ledValues[led] = FADE_INCREMENT | INCREASING_BIT;
      } else if (ledValues[led] >= MAX_VALUE) {
        ledValues[led] -= FADE_INCREMENT | INCREASING_BIT;
      } else if (ledValues[led] & INCREASING_BIT) {
        ledValues[led] += FADE_INCREMENT;
      } else {
        ledValues[led] -= FADE_INCREMENT;
        if (ledValues[led] < 0) {
          ledValues[led] = 0;
        }
      }

      if (ledValues[led] > MAX_VALUE) {
        ledValues[led] = MAX_VALUE;
      } else if (ledValues[led] < 0) {
        ledValues[led] = 0;
      }
      
      DEBUG_PRINT(led);
      DEBUG_PRINT("r");
      DEBUG_PRINT(ledValues[led]);
      DEBUG_PRINT("-");
    }
    Tlc.set(led, ledValues[led]);
  }

  row = (row + 1) % MAX_ROW;
  column = (column + 1) % MAX_COLUMN;
  
  DEBUG_PRINT("R/C:");
  DEBUG_PRINT(row);
  DEBUG_PRINT("/");
  DEBUG_PRINT(column);
  DEBUG_PRINT("\n");  

  while (Tlc.update());

  return 10;
}

/*
 * Set a random trajectory for each LED
 */
#define RANDOM_FADES_RANGE 64
int16_t random_fades_vectors[NUM_LEDS];
boolean random_fades_initialized = false;
int mode_random_fades(void *arg)
{
  if (!random_fades_initialized) {
    for (int led = 0; led < NUM_LEDS; led++) {
        random_fades_vectors[led] = -1 * (16 + random(RANDOM_FADES_RANGE));
    }
    random_fades_initialized = true;
  }

  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues[led] += random_fades_vectors[led];
    if (ledValues[led] <= 0) {
      ledValues[led] = 0;
      random_fades_vectors[led] = 16 + random(RANDOM_FADES_RANGE);
    } else if (ledValues[led] >= MAX_VALUE) {
      ledValues[led] = MAX_VALUE;
      random_fades_vectors[led] *= -1;
    } 
    Tlc.set(led, ledValues[led]);
  }

  while (Tlc.update());

  return 10;
}
