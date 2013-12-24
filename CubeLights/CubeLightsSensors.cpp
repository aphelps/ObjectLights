/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/


#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include <Arduino.h>
#include <NewPing.h>
#include <Wire.h>
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

    DEBUG_VALUE(DEBUG_HIGH, F(" Ping cm:"), range_cm);
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
    DEBUG_VALUE(DEBUG_HIGH, F(" Photo:"), photo_value);
  }
}

/* ***** Capacitive Sensors ***************************************************/

boolean touch_states[MPR121::MAX_SENSORS];
MPR121 touch_sensor; // MPR121 must be initialized after Wire.begin();

void sensor_cap_init() 
{
  Wire.begin();

  touch_sensor = MPR121(CAP_TOUCH_PIN, touch_states, false); // XXX - Problem?
  DEBUG_PRINTLN(DEBUG_MID, "Initializing cap touch");
  for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
    touch_states[i] = 0;
  }
  touch_sensor.setThreshold(0, 1, 5);
  touch_sensor.setThreshold(1, 1, 5);
}

void sensor_cap(void) 
{
  if (touch_sensor.useInterrupt) {
    if (!touch_sensor.triggered) return;
  } else {
    /* Determine if its time to perform a check */
    static long next_check = millis();
    long now = millis();
    if (now < next_check) return;
    next_check = now + CAP_DELAY_MS;
  }

  if (touch_sensor.readTouchInputs()) {
    DEBUG_PRINT(DEBUG_HIGH, F("Cap:"));
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      DEBUG_VALUE(DEBUG_HIGH, F(" "), touch_states[i]);
    }
    DEBUG_PRINT_END();
  }
}
