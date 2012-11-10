/*
 * This code drive's Adam's Umbrella sign.
 *
 * Internal to the sign is an Arduino Nano v3 driving two serially connected
 * TLC5940 LED drivers for the individual pixels of the sign.
 */

#include "Arduino.h"

#include "Tlc5940.h"
#include "tlc_shifts.h"

#if NUM_TLCS != 2
  /* NUM_TLCS must be set to 2 in tlc_config.h */
  NUM_TLCS must equal 2;
#endif

/* Array to map the light positions in the sign to the LED driver pins */
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
#define NUM_LEDS 28
uint16_t ledValues[NUM_LEDS] = {
  4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
  4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
  4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
  4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
};


/* Period to delay between shifts, in milliseconds */
#define LED_PERIOD 10

void setup()
{
  /* Initialize the LED drivers with all-on */ 
  Tlc.init(4095);

  /* Initialize the LED values */
  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues = 4095;
  }
}

int mode = 0;
void loop()
{
  /* Modify the LED values based on the current mode */
  switch (mode) {
      case 0: 
      {
        
        break;
      }
    
  }

  /* Update all TLC
  for (int led = 0; led < NUM_LEDS; led++) {
    Tlc.set(led, ledValues[led]);
  }
  while (Tlc.update()) // Wait until the data has been sent to the TLCs;

  delay(LED_PERIOD);
}

