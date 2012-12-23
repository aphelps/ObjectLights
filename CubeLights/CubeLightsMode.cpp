#define DEBUG_LEVEL DEBUG_MID
#include <Debug.h>

#include <Arduino.h>

#include "Tlc5940.h"
#include "tlc_shifts.h"
#include "tlc_fades.h"

#include "CubeLights.h"

/* Array of modes that are valid for normal use */
uint8_t validModes[] = {
//  MODE_SENSE_DISTANCE,
//  MODE_RANDOM_FADES,
  MODE_SWAP_ONE,
//  MODE_FADE_ONE,
  MODE_ALL_ON,
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
    DEBUG_VALUELN(2, "New mode: ", current_mode);
    previous_mode = current_mode;

    Tlc.setAll(0);
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

#if 0
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
#endif

/* Set all LEDs to their max value */
int mode_all_on(void *arg) 
{
  int new_value;
  if (arg != NULL) {
    new_value = (int)arg - 1;
    if (new_value < 0) new_value = 0;
    if (new_value > MAX_VALUE) new_value = MAX_VALUE;
    Serial.print("AllOn:");
    Serial.println(new_value);
  } else {
    new_value = MAX_VALUE;
  }
  if (ledValues[0] != new_value) {
    for (int led = 0; led < NUM_LEDS; led++) {
      ledValues[led] = new_value;
    }
    Tlc.setAll(new_value);

    while (Tlc.update());
  }
  
  return 0;
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
     00,
  00,   00,
     00,
  00,   00,
     
  00,   00,
     00,
  00,   00,
     00,
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

#define MIN_DISTANCE 5
#define MAX_DISTANCE 20
int mode_sense_distance(void *arg) 
{
//  long led_value = side_values[0];
//  if (led_value > MAX_VALUE) led_value = MAX_VALUE;

  int led_value;
  if (range_cm >= MAX_DISTANCE) {
    led_value = 0;
  } else if (range_cm <= MIN_DISTANCE) {
    led_value = MAX_VALUE;
  } else {
    led_value = (MAX_DISTANCE - range_cm) * MAX_VALUE /
      (MAX_DISTANCE - MIN_DISTANCE);
  }

#if 0
  Serial.print(range_cm);
  Serial.print("-");
  Serial.println(led_value);
#endif

  Tlc.setAll(led_value);
  while (Tlc.update());
  return 0;
}
