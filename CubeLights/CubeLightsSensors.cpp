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
#include "CubeConfig.h"

/* ***** Range sensor *********************************************************/

NewPing sonar(PING_TRIG_PIN, PING_ECHO_PIN, PING_MAX_CM);

uint16_t range_cm = 10;
void sensor_range(void)
{
  static unsigned long nextPing = millis();
  unsigned long now = millis();
  if (now >= nextPing) {
    nextPing = now + PING_DELAY_MS;

    uint16_t new_range = sonar.ping() / US_ROUNDTRIP_CM;
    if (new_range == 0) new_range = PING_MAX_CM;
    if (new_range != range_cm) {
      DEBUG_VALUE(DEBUG_HIGH, F(" Ping cm:"), range_cm);
      DEBUG_VALUELN(DEBUG_HIGH, F(" time:"), millis() - now);
      range_cm = new_range;
    }
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

MPR121 touch_sensor; // MPR121 must be initialized after Wire.begin();

void sensor_cap_init() 
{
  Wire.begin();

  touch_sensor = MPR121(CAP_TOUCH_IRQ, false); // XXX - Problem with interrupt?
 
  touch_sensor.setThreshold(CAP_SENSOR_1,
			    CAP_SENSOR_1_TOUCH, CAP_SENSOR_1_RELEASE);
  touch_sensor.setThreshold(CAP_SENSOR_2,
			    CAP_SENSOR_2_TOUCH, CAP_SENSOR_2_RELEASE);

  DEBUG_PRINTLN(DEBUG_MID, "Cap touch initialized");
}

void sensor_cap(void) 
{
#if 0
  if (touch_sensor.useInterrupt) {
    if (!touch_sensor.triggered) return;
  } else {
    /* Determine if its time to perform a check */
    static long next_check = millis();
    long now = millis();
    if (now < next_check) return;
    next_check = now + CAP_DELAY_MS;
  }
#endif

  if (touch_sensor.readTouchInputs()) {
    DEBUG_PRINT(DEBUG_HIGH, F("Cap:"));
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      DEBUG_VALUE(DEBUG_HIGH, F(" "), touch_sensor.touched(i));
    }
    DEBUG_VALUELN(DEBUG_HIGH, F(" ms:"), millis());
  }
}

/* ***** Handle sensor input *************************************************/

void handle_sensors() {

  /* Sensor 1 controls the mode */
  if (touch_sensor.changed(CAP_SENSOR_1) &&
      touch_sensor.touched(CAP_SENSOR_1) &&
     !touch_sensor.touched(CAP_SENSOR_2)) {
    /* The sensor was just touched */
    increment_mode();
  }

  /* Sensor 2 controls the color */
  if (touch_sensor.touched(CAP_SENSOR_2)) {
    static byte color = 0;
    color++;
    modeConfig.fgColor = pixel_wheel(color);
    //    DEBUG_VALUELN(DEBUG_HIGH, F("Color="), color);

    if (touch_sensor.touched(CAP_SENSOR_1)) {
      modeConfig.fgColor = pixel_color(255, 255, 255);
    }
  }

  if (range_cm < 50) {
    set_followup(0);
  } else {
    set_followup((byte)-1);
  }
}
