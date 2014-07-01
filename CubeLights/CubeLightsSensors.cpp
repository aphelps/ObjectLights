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
    DEBUG_PRINT(DEBUG_TRACE, "Cap:");
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      DEBUG_VALUE(DEBUG_TRACE, " ", touch_sensor.touched(i));
    }
    DEBUG_VALUELN(DEBUG_TRACE, " ms:", millis());
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
  uint32_t prev_state = sensor_state;
  sensor_state = 0;

  // Short range detected
  if (range_cm < PING_SHORT_CM) sensor_state |= SENSE_RANGE_SHORT;
  else if (range_cm < PING_MID_CM) sensor_state |= SENSE_RANGE_MID;
  else if (range_cm < PING_MAX_CM) sensor_state |= SENSE_RANGE_LONG;

  // Sensor 1
  if (touch_sensor.touched(CAP_SENSOR_1)) sensor_state |= SENSE_TOUCH_1;
  if (touch_sensor.changed(CAP_SENSOR_1)) sensor_state |= SENSE_CHANGE_1;
  if (CHECK_TAP_1()) {
    // Detect double tap on individual sensors
    static unsigned long taptime1 = 0;
    if (now - taptime1 < CAP_DOUBLE_MS) {
      sensor_state |= SENSE_DOUBLE_1;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap1 ms:", now - taptime1);
    }
    taptime1 = now;
  }
  // XXX - Detect long touch?

  // Sensor 2
  if (touch_sensor.touched(CAP_SENSOR_2)) sensor_state |= SENSE_TOUCH_2;
  if (touch_sensor.changed(CAP_SENSOR_2)) sensor_state |= SENSE_CHANGE_2;
  if (CHECK_TAP_2()) {
    // Detect double tap on individual sensors
    static unsigned long taptime2 = 0;
    if (now - taptime2 < CAP_DOUBLE_MS) {
      sensor_state |= SENSE_DOUBLE_2;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap2 ms:", now - taptime2);
    }
    taptime2 = now;
  }
  // XXX - Detect long touch?

  static unsigned long doubleTime = 0;

  // Both became touched (could be one then the other)
  if (CHECK_TAP_BOTH()) {
    static unsigned long taptime = 0;
    if (now - taptime < CAP_DOUBLE_MS) {
      // Rapid double tap
      sensor_state |= SENSE_DOUBLE_BOTH;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap ms:", now - taptime);
    }

    doubleTime = now;
    taptime = now;
  }

  // Detect a long touch on both
  if (CHECK_TOUCH_BOTH()) {
    if ((doubleTime > 0) && ((now - doubleTime) > 750)) {
      // Long touch period
      DEBUG_PRINTLN(DEBUG_HIGH, "Long double touch");
      sensor_state |= SENSE_LONG_BOTH;
      doubleTime = 0;
    }
  }

  // Neither sensor is touched
  if (CHECK_TOUCH_NONE()) {
    doubleTime = 0;
  }

  /****************************************************************************
   * Handling
   */

  if (CHECK_LONG_BOTH()) {
    uiMode = (uiMode + 1) % NUM_UI_MODES;
    DEBUG_VALUELN(DEBUG_LOW, "Enter ui mode: ", uiMode);
  }

  switch (uiMode) {
  case 0: {
    if (CHECK_LONG_BOTH()) {
      // Just entered this mode
      set_mode_to(FINAL_MODE, MODE_NONE);
      DEBUG_PRINTLN(DEBUG_HIGH, "Entered default UI");
    }

    // If just sensor 1 is being touched
    if (CHECK_TOUCH_1() && !CHECK_TOUCH_BOTH()) {
      // Set followup color
      static byte color = 0;
      color++;
      modeConfigs[FINAL_MODE].fgColor = pixel_wheel(color);
    }

    // If just sensor 2 is being touched
    if (CHECK_TOUCH_2() && !CHECK_TOUCH_BOTH()) {
      static byte color = 0;
      color++;
      modeConfigs[0].fgColor = pixel_wheel(color);
    }

    if (CHECK_TAP_BOTH()) {
      modeConfigs[0].fgColor = pixel_color(255, 255, 255);
    }

    if (CHECK_DOUBLE_BOTH()) {
      increment_mode(0);
    }

    if (CHECK_RANGE_SHORT()) {
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
#if 0
  case 1: {
    if (CHECK_LONG_BOTH()) {
      // Just entered mode changing state
      modeConfigs[0].fgColor = pixel_color(255, 255, 255);
      modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 0, 255);
      modeConfigs[FINAL_MODE].data.u32s[0] =
	FACE_LED_MASK(0xFF, ((1 << 0) | (1 << 2) | (1 << 6) | (1 << 8)));
      set_mode_to(FINAL_MODE, MODE_BLINK_PATTERN);
      DEBUG_PRINTLN(DEBUG_LOW, "Entered mode change");
    }

    if (CHECK_TAP_1() && !CHECK_TOUCH_BOTH()) {
      increment_mode(0);
    }

    if (CHECK_TAP_2() && !CHECK_TOUCH_BOTH()) {
      modeConfigs[FINAL_MODE].fgColor = pixel_color(255, 0, 0);
      increment_mode(FINAL_MODE);
    }
    
    break;
  }
#endif
  case 1: {
    /*
     * This mode is the remote control for a simple flame effect.
     * - sensor 1: while held opens solenoid
     * - sensor 2: tapped for brief open
     */

#define IGNITER_OUTPUT 0
#define PILOT_VALVE 1
#define POOF_OUTPUT 2

#define REFRESH_PERIOD 150
    static unsigned long last_send = 0;

#define PILOT_OFF      0
#define PILOT_IGNITING 1
#define PILOT_ON       2
    static byte pilot_state = PILOT_OFF;

    if (CHECK_LONG_BOTH()) {
      // Just entered mode changing state

      if (pilot_state == PILOT_OFF) {
	/* If the pilot hadn't previously been ignited, do so now */
	DEBUG_PRINTLN(DEBUG_HIGH, "Igniting pilot light");

	/* Turn on the hot surface igniter */
	sendHMTLTimedChange(ADDRESS_POOFER_UNIT, IGNITER_OUTPUT,
			    30000, 0xFFFFFFFF, 0);
	last_send = now;
	
	modeConfigs[FINAL_MODE].fgColor = pixel_color(255, 0, 0);
      } else {
	modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 255, 0);
      }

      modeConfigs[FINAL_MODE].data.u32s[0] =
	FACE_LED_MASK(0xFF, ((1 << 6) | (1 << 7) | (1 << 8)));
      set_mode_to(FINAL_MODE, MODE_BLINK_PATTERN);
      DEBUG_HEXVALLN(DEBUG_HIGH, "Entered poofer control.  Mask:", 
		     modeConfigs[FINAL_MODE].data.u32s[0]);
    }

    if (pilot_state == PILOT_OFF) {
      if (now - last_send > 5000) {
	/* Once the igniter has had time to warm up, open the pilot valve */
	sendHMTLValue(ADDRESS_POOFER_UNIT, PILOT_VALVE, 255);
	pilot_state = PILOT_IGNITING;
	modeConfigs[FINAL_MODE].data.u32s[0] =
	  FACE_LED_MASK(0xFF, ((1 << 6) | (1 << 8)));

	DEBUG_PRINTLN(DEBUG_HIGH, "Opening pilot light value");
      }
    } else if (pilot_state == PILOT_IGNITING) {
      if (now - last_send > 30000) {
	/* The pilot light should be triggered by this point */
	pilot_state = PILOT_ON;
	modeConfigs[FINAL_MODE].data.u32s[0] =
	  FACE_LED_MASK(0xFF, ((1 << 6) | (1 << 7) | (1 << 8)));
	modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 255, 0);

	DEBUG_PRINTLN(DEBUG_HIGH, "Flame on!!!");
      }
    } else {
      if (CHECK_TOUCH_1() && !CHECK_TOUCH_BOTH()) {
	if (CHECK_CHANGE_1()) {
	  last_send = 0;
	}

	if (now - last_send > REFRESH_PERIOD) {
	  /* Send a brief on value, repeated calls will keep the valve open */
	  sendHMTLTimedChange(ADDRESS_POOFER_UNIT, POOF_OUTPUT,
			      250, 0xFFFFFFFF, 0);
	  last_send = now;
	  DEBUG_PRINTLN(DEBUG_HIGH, "Sending poof");
	}
      } else if (CHECK_CHANGE_1() && !CHECK_TOUCH_BOTH()) {
	/* The sensor was released, send a disable message */
	// XXX - Add program cancel message
	sendHMTLTimedChange(ADDRESS_POOFER_UNIT, POOF_OUTPUT,
			    10, 0, 0);
	last_send = now;
	  DEBUG_PRINTLN(DEBUG_HIGH, "Disable poof");
      } else if (CHECK_TOUCH_2() && CHECK_CHANGE_2()) {
	/* Send a stutter open */
	sendHMTLTimedChange(ADDRESS_POOFER_UNIT, POOF_OUTPUT,
			    100, 0xFFFFFFFF, 0);
	last_send = now;
	DEBUG_PRINTLN(DEBUG_HIGH, "Quick poof");
      }
    }
  }
  }

#ifdef ADDRESS_RECV_TEST
  if (CHECK_TOUCH_ANY()) {
    /* A sensor is touched, send update to remotes */
    static unsigned long next_send = millis();
    if (now >= next_send) {
      int command = 0;
      if (CHECK_TOUCH_1()) command |= (1 << CAP_SENSOR_1);
      if (CHECK_TOUCH_2()) command |= (1 << CAP_SENSOR_2);

      sendInt(command, ADDRESS_RECV_TEST);
      next_send = now + 100;
    }
  } else if (CHECK_CHANGE_ANY()) {
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

#endif
}
