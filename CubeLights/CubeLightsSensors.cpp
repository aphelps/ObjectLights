/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include <Arduino.h>
#include <NewPing.h>
#include <Wire.h>
#include "MPR121.h"

#include "CubeLights.h"
#include "CubeConfig.h"
#include "SquareStructure.h"

/* ***** Range sensor *********************************************************/

NewPing sonar(PING_TRIG_PIN, PING_ECHO_PIN, PING_MAX_CM);

int range_cm = 10;
void sensor_range(void)
{
  static unsigned long nextPing = millis();
  unsigned long now = millis();
  if (now >= nextPing) {
    nextPing = now + PING_DELAY_MS;

    int new_range = sonar.ping_cm();
    if (new_range == 0) new_range = PING_MAX_CM;
    if (new_range != range_cm) {
      DEBUG_COMMAND(DEBUG_HIGH,
		    if (abs(new_range - range_cm) > 5) {
		      DEBUG_VALUE(DEBUG_HIGH, " Ping cm:", new_range);
		      DEBUG_VALUE(DEBUG_HIGH, " old_cm:", range_cm);
		      DEBUG_VALUELN(DEBUG_HIGH, " time:", millis() - now);
		    }
		    );
      range_cm = new_range;
    }
  }
}

/* ***** Photo sensor *********************************************************/

#if 0
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
#endif

/* ***** Capacitive Sensors ***************************************************/

MPR121 touch_sensor; // MPR121 must be initialized after Wire.begin();

#if 0
void sensor_cap_init() 
{
  Wire.begin();

  touch_sensor = MPR121(CAP_TOUCH_IRQ, false, false); // XXX - Problem with interrupt?
 
  touch_sensor.setThreshold(CAP_SENSOR_1,
			    CAP_SENSOR_1_TOUCH, CAP_SENSOR_1_RELEASE);
  touch_sensor.setThreshold(CAP_SENSOR_2,
			    CAP_SENSOR_2_TOUCH, CAP_SENSOR_2_RELEASE);
  DEBUG_PRINTLN(DEBUG_MID, "Cap touch initialized");
}
#endif

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
    DEBUG_PRINT(DEBUG_HIGH, "Cap:");
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      DEBUG_VALUE(DEBUG_HIGH, " ", touch_sensor.touched(i));
    }
    DEBUG_VALUELN(DEBUG_HIGH, " ms:", millis());
  }
}

/* ***** Handle sensor input *************************************************/

#define SENSOR_MODE 1

uint32_t sensor_state = 0;

void handle_sensors() {
  unsigned long now = millis();

#if SENSOR_MODE == 0
#if 0
  if (touch_sensor.touched(CAP_SENSOR_1) ||
      (touch_sensor.touched(CAP_SENSOR_2))) {
    /* A sensor is touched, send update to remotes */
    static unsigned long next_send = millis();
    if (now >= next_send) {
      int command =
	(touch_sensor.touched(CAP_SENSOR_1) << CAP_SENSOR_1) |
	(touch_sensor.touched(CAP_SENSOR_2) << CAP_SENSOR_2);
      sendInt(command, ADDRESS_RECV_TEST);
      next_send = now + 100;
    }
  } else if (touch_sensor.changed(CAP_SENSOR_1) ||
	     touch_sensor.changed(CAP_SENSOR_2)) {
    sendInt(0, ADDRESS_RECV_TEST);
  }
#endif

  /* Sensor 1 controls the mode */
  if (touch_sensor.changed(CAP_SENSOR_1) &&
      touch_sensor.touched(CAP_SENSOR_1) &&
     !touch_sensor.touched(CAP_SENSOR_2)) {
    /* The sensor was just touched on its own */
    increment_mode();
  }

  /* Sensor 2 controls the color */
  if (touch_sensor.touched(CAP_SENSOR_2)) {
    static byte color = 0;
    color++;
    modeConfig.fgColor = pixel_wheel(color);
    //    DEBUG_VALUELN(DEBUG_HIGH, "Color=", color);

    if (touch_sensor.touched(CAP_SENSOR_1)) {
      modeConfig.fgColor = pixel_color(255, 255, 255);
    }
  }

  /* Not both sensors */
  if (!(touch_sensor.touched(CAP_SENSOR_1) &&
	touch_sensor.touched(CAP_SENSOR_2))) {
    /*    switch (get_current_followup()) {
    case MODE_BLINK_FACE:
      set_followup((byte)-1);
      break;
      }*/
  }

  if (range_cm < 50) {
    if (get_current_mode(FINAL_MODE) != MODE_LIGHT_CENTER) {
      set_mode_to(FINAL_MODE, MODE_LIGHT_CENTER);
    }
  } else {
    if (get_current_mode(FINAL_MODE) == MODE_LIGHT_CENTER) {
      restore_mode(FINAL_MODE);
    }
  }
#endif
#if SENSOR_MODE == 1
#define NUM_UI_MODES 2
  static byte uiMode = 0;

  /****************************************************************************
   * State determination
   */
  uint32_t state = 0;

  // Short range detected
  if (range_cm < PING_SHORT_CM) state |= SENSE_RANGE_SHORT;
  else if (range_cm < PING_MID_CM) state |= SENSE_RANGE_MID;
  else if (range_cm < PING_MAX_CM) state |= SENSE_RANGE_LONG;

  // Sensor 1
  if (touch_sensor.touched(CAP_SENSOR_1)) state |= SENSE_TOUCH_1;
  if (touch_sensor.changed(CAP_SENSOR_1)) state |= SENSE_CHANGE_1;
  if (CHECK_TAP_1(state)) {
    // Detect double tap on individual sensors
    static unsigned long taptime1 = 0;
    if (now - taptime1 < CAP_DOUBLE_MS) {
      state |= SENSE_DOUBLE_1;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap1 ms:", now - taptime1);
    }
    taptime1 = now;
  }
  // XXX - Detect long touch?

  // Sensor 2
  if (touch_sensor.touched(CAP_SENSOR_2)) state |= SENSE_TOUCH_2;
  if (touch_sensor.changed(CAP_SENSOR_2)) state |= SENSE_CHANGE_2;
  if (CHECK_TAP_2(state)) {
    // Detect double tap on individual sensors
    static unsigned long taptime2 = 0;
    if (now - taptime2 < CAP_DOUBLE_MS) {
      state |= SENSE_DOUBLE_2;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap2 ms:", now - taptime2);
    }
    taptime2 = now;
  }
  // XXX - Detect long touch?

  static unsigned long doubleTime = 0;

  // Both became touched (could be one then the other)
  if (CHECK_TAP_BOTH(state)) {
    static unsigned long taptime = 0;
    if (now - taptime < CAP_DOUBLE_MS) {
      // Rapid double tap
      state |= SENSE_DOUBLE_BOTH;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap ms:", now - taptime);
    }

    doubleTime = now;
    taptime = now;
  }

  // Detect a long touch on both
  if (CHECK_TOUCH_BOTH(state)) {
    if ((doubleTime > 0) && ((now - doubleTime) > 750)) {
      // Long touch period
      DEBUG_PRINTLN(DEBUG_HIGH, "Long double touch");
      state |= SENSE_LONG_BOTH;
      doubleTime = 0;
    }
  }

  // Neither sensor is touched
  if (CHECK_TOUCH_NONE(state)) {
    doubleTime = 0;
  }

  sensor_state = state;

  /****************************************************************************
   * Handling
   */

  if (CHECK_LONG_BOTH(state)) {
    uiMode = (uiMode + 1) % NUM_UI_MODES;
  }

  switch (uiMode) {
  case 0: {
    // If just sensor 1 is being touched
    if (CHECK_TOUCH_1(state) && !CHECK_TOUCH_BOTH(state)) {
      // Set followup color
      static byte color = 0;
      color++;
      modeConfigs[FINAL_MODE].fgColor = pixel_wheel(color);
    }

    // If just sensor 2 is being touched
    if (CHECK_TOUCH_2(state) && !CHECK_TOUCH_BOTH(state)) {
      static byte color = 0;
      color++;
      modeConfigs[0].fgColor = pixel_wheel(color);
    }

    if (CHECK_TAP_BOTH(state)) {
      modeConfigs[0].fgColor = pixel_color(255, 255, 255);
    }

    if (CHECK_DOUBLE_BOTH(state)) {
      increment_mode(0);
    }

    if (CHECK_RANGE_SHORT(state)) {
      //if (get_current_mode(FINAL_MODE) != MODE_LIGHT_CENTER) {
      if (get_current_mode(FINAL_MODE) != MODE_STATIC_NOISE) {
	modeConfigs[FINAL_MODE].fgColor = pixel_color(255, 0, 0);
	//set_mode_to(FINAL_MODE, MODE_LIGHT_CENTER);
	//  XXX set_mode_to(FINAL_MODE, MODE_STATIC_NOISE);
      }
    } else {
      //if (get_current_mode(FINAL_MODE) == MODE_LIGHT_CENTER) {
      if (get_current_mode(FINAL_MODE) == MODE_STATIC_NOISE) {
	restore_mode(FINAL_MODE);
      }
    }
    break;
  }
  case 1: {
    if (CHECK_LONG_BOTH(state)) {
      // Just entered mode changing state
      modeConfigs[0].fgColor = pixel_color(255, 255, 255);
      modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 0, 255);
      modeConfigs[FINAL_MODE].data.u32s[0] =
	FACE_LED_MASK(0xFF, ((1 << 0) | (1 << 2) | (1 << 6) | (1 << 8)));
      set_mode_to(FINAL_MODE, MODE_BLINK_PATTERN);
      DEBUG_PRINTLN(DEBUG_HIGH, "Entered mode change");
    }

    if (CHECK_TAP_1(state) && !CHECK_TOUCH_BOTH(state)) {
      increment_mode(0);
    }

    if (CHECK_TAP_2(state) && !CHECK_TOUCH_BOTH(state)) {
      modeConfigs[FINAL_MODE].fgColor = pixel_color(255, 0, 0);
      increment_mode(FINAL_MODE);
    }
    
    break;
  }
  }

#ifdef ADDRESS_RECV_TEST
  if (CHECK_TOUCH_ANY(state)) {
    /* A sensor is touched, send update to remotes */
    static unsigned long next_send = millis();
    if (now >= next_send) {
      int command = 0;
      if (CHECK_TOUCH_1(state)) command |= (1 << CAP_SENSOR_1);
      if (CHECK_TOUCH_2(state)) command |= (1 << CAP_SENSOR_2);

      sendInt(command, ADDRESS_RECV_TEST);
      next_send = now + 100;
    }
  } else if (CHECK_CHANGE_ANY(state)) {
    /* Touch was removed */
    sendInt(0, ADDRESS_RECV_TEST);
  }
#endif // ADDRESS_RECV_TEST

#ifdef ADDRESS_TRIANGLES
  // Send the current color
  static unsigned long next_send = millis();
  if (now >= next_send) {
    sendLong(modeConfigs[0].fgColor, ADDRESS_TRIANGLES);
    next_send = now + 50;
  }
#endif // ADDRESS_TRIANGLES

#ifdef ADDRESS_TRIGGER_UNIT
  static unsigned long next_send = millis();
  if ((now >= next_send) && (CHECK_TOUCH_ANY(state))) {
    int value = 0;
    if (CHECK_TOUCH_1(state)) value = 255;
    sendHMTLValue(ADDRESS_TRIGGER_UNIT, 0, value)
    next_send = now + 50;
  }
#endif

#endif
}
