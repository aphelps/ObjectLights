/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

//#define DEBUG_LEVEL DEBUG_TRACE
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
      DEBUG4_COMMAND(
                     if (abs(new_range - range_cm) > 5) {
                       DEBUG4_VALUE(" Ping cm:", new_range);
                       DEBUG4_VALUE(" old_cm:", range_cm);
                       DEBUG4_VALUELN(" time:", millis() - now);
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
    DEBUG4_VALUE(" Photo:", photo_value);
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
    DEBUG5_COMMAND(
                   DEBUG5_PRINT("Cap:");
                   for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
                     DEBUG5_VALUE(" ", touch_sensor.touched(i));
                   }
                   DEBUG5_VALUELN(" ms:", millis());
                   );
  }
}

/* ***** Handle sensor input *************************************************/

/* Bit mask of the complete sensor state */
uint32_t sensor_state = 0;

sensor_mode_t sensorFunctions[] = {
  sensor_mode_basic_control,
  //sensor_mode_mode_control
  sensor_mode_poofer_control
};
#define NUM_UI_MODES (sizeof (sensorFunctions) / sizeof (sensor_mode_t))

void handle_sensors() {
  unsigned long now = millis();

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
      DEBUG4_VALUELN("Double tap1 ms:", now - taptime1);
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
      DEBUG4_VALUELN("Double tap2 ms:", now - taptime2);
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
      DEBUG4_VALUELN("Double tap ms:", now - taptime);
    }

    doubleTime = now;
    taptime = now;
  }

  // Detect a long touch on both
  if (CHECK_TOUCH_BOTH()) {
    if ((doubleTime > 0) && ((now - doubleTime) > 750)) {
      // Long touch period
      DEBUG4_PRINTLN("Long double touch");
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
    /*
     * A long touch on both sensors indicates a mode change
     */

    // Send exit to current function
    sensorFunctions[uiMode](false, true);

    uiMode = (uiMode + 1) % NUM_UI_MODES;
    DEBUG2_VALUELN("Enter ui mode: ", uiMode);

    // Send enter to new function
    sensorFunctions[uiMode](true, false);
  } else {
    sensorFunctions[uiMode](false, false);
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
}

void sensor_mode_basic_control(boolean entered, boolean exited) {
  if (exited) {
    return;
  }

  if (entered) {
    // Just entered this mode, clear followup modes
    set_mode_to(FINAL_MODE, MODE_NONE);
    DEBUG4_PRINTLN("Entered default UI");
  }

  // If only one sensor is touched, adjust the color
  if ((CHECK_TOUCH_1() || CHECK_TOUCH_2()) && !CHECK_TOUCH_BOTH()) {
    static byte color = 0;
    if (CHECK_TOUCH_1()) {
      color++;
    } else {
      color--;
    }
    modeConfigs[0].fgColor = pixel_wheel(color);
  }

  if (CHECK_TAP_BOTH()) {
    modeConfigs[0].fgColor = pixel_color(255, 255, 255);
  }

  if (CHECK_DOUBLE_BOTH()) {
    increment_mode(0);
  }

#if 0
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
#endif
}

void sensor_mode_poofer_control(boolean entered, boolean exited) {
  /*
   * This mode is the remote control for a simple flame effect.
   * - sensor 1: while held opens solenoid
   * - sensor 2: tapped for brief open
   */

#define IGNITER_OUTPUT 0
#define PILOT_VALVE    1
#define POOF_OUTPUT    2
#define LIGHT_OUTPUT   3

#define REFRESH_PERIOD 150
  static unsigned long last_send = 0;
  unsigned long now = millis();

#define PILOT_OFF      0
#define PILOT_IGNITING 1
#define PILOT_ON       2
  static byte pilot_state = PILOT_OFF;

  if (exited) {
    if (pilot_state == PILOT_ON) {
      // Set the external light to dim
      sendHMTLValue(ADDRESS_POOFER_UNIT, LIGHT_OUTPUT, 8);
    } else {
      /*
       * State was changed while in ignition sequence, send command to
       * turn off the ignitor and pilot valve
       */
      //XXX - Clear program, or set value to off?

      // Set the external light to off
      sendHMTLValue(ADDRESS_POOFER_UNIT, LIGHT_OUTPUT, 0);
    }

    return;
  }

  if (entered) {
    // Just entered mode changing state

    // Turn the external light to max
    sendHMTLValue(ADDRESS_POOFER_UNIT, LIGHT_OUTPUT, 255);

    if (pilot_state == PILOT_OFF) {
      /* If the pilot hadn't previously been ignited, do so now */
      DEBUG4_PRINTLN("Igniting pilot light");

      delay(10); // XXX - For some reason we need a delay between transmissions

      /* Turn on the hot surface igniter */
      sendHMTLTimedChange(ADDRESS_POOFER_UNIT, IGNITER_OUTPUT,
                          30000, 0xFFFFFFFF, 0);

      last_send = now;
	
      modeConfigs[FINAL_MODE].fgColor = pixel_color(255, 0, 0);
    } else {
      modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 255, 0);
    }

    modeConfigs[FINAL_MODE].data.u32s[0] = 0x0 | 
      FACE_LED_MASK(0x0F, ((1 << 6) | (1 << 7) | (1 << 8)));
    set_mode_to(FINAL_MODE, MODE_BLINK_PATTERN);
    DEBUG4_HEXVALLN("Entered poofer control.  Mask:", 
                    modeConfigs[FINAL_MODE].data.u32s[0]);
  }

  if (pilot_state == PILOT_OFF) {
    if (now - last_send > 5000) {
      /* Once the igniter has had time to warm up, open the pilot valve */
      sendHMTLValue(ADDRESS_POOFER_UNIT, PILOT_VALVE, 255);
      pilot_state = PILOT_IGNITING;
      modeConfigs[FINAL_MODE].data.u32s[0] =
        FACE_LED_MASK(0xFF, ((1 << 6) | (1 << 8)));

      DEBUG4_PRINTLN("Opening pilot light value");
    }
  } else if (pilot_state == PILOT_IGNITING) {
    if (now - last_send > 30000) {
      /* The pilot light should be triggered by this point */
      pilot_state = PILOT_ON;
      modeConfigs[FINAL_MODE].data.u32s[0] =
        FACE_LED_MASK(0xFF, ((1 << 6) | (1 << 7) | (1 << 8)));
      modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 255, 0);

      DEBUG4_PRINTLN("Flame on!!!");
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
        DEBUG4_PRINTLN("Sending poof");
      }
    } else if (CHECK_CHANGE_1() && !CHECK_TOUCH_BOTH()) {
      /* The sensor was released, send a disable message */
      // XXX - Add program cancel message
      sendHMTLTimedChange(ADDRESS_POOFER_UNIT, POOF_OUTPUT,
                          10, 0, 0);
      last_send = now;
      DEBUG4_PRINTLN("Disable poof");
    } else if (CHECK_TOUCH_2() && CHECK_CHANGE_2()) {
      /* Send a stutter open */
      sendHMTLTimedChange(ADDRESS_POOFER_UNIT, POOF_OUTPUT,
                          100, 0xFFFFFFFF, 0);
      last_send = now;
      DEBUG4_PRINTLN("Quick poof");
    }
  }

}

void sensor_mode_mode_control(boolean entered, boolean exited) {
  if (exited) {
    return;
  }

  if (exited) {
    // Just entered mode changing state
    modeConfigs[0].fgColor = pixel_color(255, 255, 255);
    modeConfigs[FINAL_MODE].fgColor = pixel_color(0, 0, 255);
    modeConfigs[FINAL_MODE].data.u32s[0] =
      FACE_LED_MASK(0xFF, ((1 << 0) | (1 << 2) | (1 << 6) | (1 << 8)));
    set_mode_to(FINAL_MODE, MODE_BLINK_PATTERN);
    DEBUG2_PRINTLN("Entered mode change");
  }

  if (CHECK_TAP_1() && !CHECK_TOUCH_BOTH()) {
    increment_mode(0);
  }

  if (CHECK_TAP_2() && !CHECK_TOUCH_BOTH()) {
    modeConfigs[FINAL_MODE].fgColor = pixel_color(255, 0, 0);
    increment_mode(FINAL_MODE);
  }
}
