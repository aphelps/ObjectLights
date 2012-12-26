/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/


#define DEBUG_LEVEL 2
#include <Debug.h>

#include <Arduino.h>
#include <NewPing.h>
#include <CapacitiveSensor.h>

#include "CubeLights.h"

/* ***** Range sensor *********************************************************/

NewPing sonar(PING_TRIG_PIN, PING_ECHO_PIN, PING_MAX_CM);

uint16_t range_cm = 10;
void sensor_range(void)
{
  static unsigned long nextPing = millis();
  unsigned long now = millis();
  if (now >= nextPing) {
    nextPing = now + PING_DELAY_MS;
    range_cm = sonar.ping() / US_ROUNDTRIP_CM;

    DEBUG_VALUE(2, " Ping:", range_cm);
  }
}

/* ***** Photo sensor *********************************************************/

uint16_t photo_value = 1024;
boolean photo_dark = false;
void sensor_photo(void)
{
  static unsigned long next_photo_sense = millis();
  unsigned long now = millis();

  if (now > next_photo_sense) {
    next_photo_sense = now + PHOTO_DELAY_MS;

    photo_value = analogRead(PHOTO_PIN);
    if (photo_value > PHOTO_THRESHOLD_HIGH) {
      photo_dark = false;
    } else if (photo_value < PHOTO_THRESHOLD_LOW) {
      photo_dark = false;
    }
    DEBUG_VALUE(2, " Photo:", photo_value);
  }
}

/* ***** Capacitive Sensors ***************************************************/

CapacitiveSensor cap_sensors[NUM_CAP_SENSORS] = {
  CapacitiveSensor(4,5),
  CapacitiveSensor(4,6),
};
long cap_values[NUM_CAP_SENSORS];
long cap_min[NUM_CAP_SENSORS];
long cap_max[NUM_CAP_SENSORS];

void sensor_cap_init() 
{
  for (uint8_t cap = 0; cap < NUM_CAP_SENSORS; cap++) {
    cap_sensors[cap].set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
    cap_sensors[cap].set_CS_Timeout_Millis(100);
    cap_min[cap] = 0xFFFF;
    cap_max[cap] = 0;
  }
}

void sensor_cap(void) 
{
  /* Determine if its time to perform a check */
  static long next_check = millis();
  long now = millis();
  if (now < next_check) return;
  next_check = now + CAP_DELAY_MS;
  
  /* Read the cap sensors */
  long start = millis();
  long sense_delay;
  for (int cap = 0; cap < NUM_CAP_SENSORS; cap++) {
    //long value = cap_sensors[cap].capacitiveSensor(10);
    long value = cap_sensors[cap].capacitiveSensorRaw(1);
    if (value < cap_min[cap]) cap_min[cap] = value;
    if (value > cap_max[cap]) cap_max[cap] = value;

    cap_values[cap] = value;
//    cap_values[cap] = map(value,
//                            cap_min[cap], cap_max[cap],
//                            0, MAX_VALUE);
  }
  DEBUG_PRINT(2, " Cap: ");
  sense_delay = millis() - start;
  DEBUG_PRINT(2, sense_delay);        // check on performance in milliseconds
  DEBUG_PRINT(2, "\t");                    // tab character for debug windown spacing
  for (int cap = 0; cap < NUM_CAP_SENSORS; cap++) {
    DEBUG_PRINT(2, cap_values[cap]);
    DEBUG_PRINT(2, "-");
    DEBUG_PRINT(2, log(cap_values[cap]));
    DEBUG_PRINT(2, "    ");
  }
}
