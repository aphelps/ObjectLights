/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/


#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include <Arduino.h>
#include <NewPing.h>
#include <CapacitiveSensor.h>
#include "MPR121.h"

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

    DEBUG_VALUE(DEBUG_HIGH, " Ping:", range_cm);
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
    DEBUG_VALUE(DEBUG_HIGH, " Photo:", photo_value);
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

MPR121 touch_sensor(CAP_TOUCH_PIN);
boolean touch_states[CAP_TOUCH_MAX];

void sensor_cap_init() 
{
  for (byte i = 0; i < CAP_TOUCH_MAX; i++) {
    touch_states[i] = 0;
  }
}

void sensor_cap(void) 
{
  /* Determine if its time to perform a check */
  static long next_check = millis();
  long now = millis();
  if (now < next_check) return;
  next_check = now + CAP_DELAY_MS;

  touch_sensor.readTouchInputs(touch_states);
  DEBUG_PRINT(DEBUG_HIGH, "Cap:");
  for (byte i = 0; i < CAP_TOUCH_MAX; i++) {
    DEBUG_VALUE(DEBUG_HIGH, " ", touch_states[i]);
  }
  DEBUG_PRINT_END();
}
