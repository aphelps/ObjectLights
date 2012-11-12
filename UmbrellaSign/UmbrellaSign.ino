/*
    A circular light buffer.  If you manage to construct a circle of LEDs,
    definitely send in pictures.  What this sketch does is take an analog
    reading off of analog pin 0 and add it to the current value of the last LED.
    If the resultant sum is greater than 4095, it turns the LED off,
    otherwise sets LED 0 to the value of the sum.

    If you ground pin 12, it will set LED 0 to zero.

    Then it shifts all the LED values up one (so LED 0 becomes LED 1) and sets
    LED 0 to the value shifted off the last LED (so if one LED is on, it will
    go in a circle forever).

    See the BasicUse example for hardware setup.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-04 */

#include "Arduino.h"

#include "Tlc5940.h"
#include "tlc_shifts.h"
#include "tlc_fades.h"

// which analog pin to use
#define ANALOG_PIN      0

// which pin to clear the LEDs with
#define CLEAR_PIN      12

#if NUM_TLCS != 2
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
#define MAX_VALUE 4095
#define NUM_LEDS 28
uint16_t ledValues[NUM_LEDS] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
};

/* Mode for lights */
#define MODE_EXAMPLE_CIRCULAR 1
#define MODE_EXAMPLE_FADES    2
#define MODE_ALL_ON           3
#define MODE_SWAP_ONE         4
int mode = MODE_SWAP_ONE;

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


void loop()
{
  int led_period = 0;

  /* Modify the LED values based on the current mode */
  switch (mode) {
      case MODE_EXAMPLE_CIRCULAR: 
      {
        /* From TCL5940 Library's CircularLightBuffer example */
        uint16_t sum = tlc_shiftUp() + 512 * 4;
        if (sum > MAX_VALUE)
          sum = 0;
        Tlc.set(0, sum);
        led_period = (2000 / 16);
        break;
      }
      case MODE_EXAMPLE_FADES:
      {
        /* From TCL5940 Library's Fades example */
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
        return;
      }
      case MODE_ALL_ON:
      {
        /* Update all TLC */
        for (int led = 0; led < NUM_LEDS; led++) {
          ledValues[led] = MAX_VALUE;
          Tlc.set(led, ledValues[led]);
        }
        led_period = 1000;
        break;
      }
      case MODE_SWAP_ONE:
      {
        /* Swap the state of a single LED */
        int led = random(NUM_LEDS);
        if (ledValues[led]) ledValues[led] = 0;
        else ledValues[led] = MAX_VALUE;
        Tlc.set(led, ledValues[led]);
        led_period = 100;
        break;
      }
  }

  while (Tlc.update()) // Wait until the data has been sent to the TLCs;

  delay(led_period);
}

