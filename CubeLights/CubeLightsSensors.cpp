/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
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

void handle_sensors() {
  unsigned long now = millis();

#if 0
  if (touch_sensor.touched(CAP_SENSOR_1) ||
      (touch_sensor.touched(CAP_SENSOR_2))) {
    /* A sensor is touched, send update to remotes */
    static unsigned long next_send = millis();
    if (now >= next_send) {
      int command =
	(touch_sensor.touched(CAP_SENSOR_1) << CAP_SENSOR_1) |
	(touch_sensor.touched(CAP_SENSOR_2) << CAP_SENSOR_2);
      sendInt(command);
      next_send = now + 100;
    }
  } else if (touch_sensor.changed(CAP_SENSOR_1) ||
	     touch_sensor.changed(CAP_SENSOR_2)) {
    sendInt(0);
  }

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
      set_followup(1);
      //modeConfig.fgColor = pixel_color(255, 255, 255);
    }
  }

  /* Not both sensors */
  if (!(touch_sensor.touched(CAP_SENSOR_1) &&
	touch_sensor.touched(CAP_SENSOR_2))) {
    switch (get_current_followup()) {
    case MODE_BLINK_FACE:
      set_followup((byte)-1);
      break;
    }
  }

  if (range_cm < 50) {
    set_followup(0);
  } else {
    if (get_current_followup() == MODE_LIGHT_CENTER) set_followup((byte)-1);
  }
#else
#define NUM_UI_MODES 2
  static byte uiMode = 0;

  boolean cap1 = false, cap2 = false, capboth = false, capnone = false;
  boolean change1 = false, change2 = false, changeboth = false,
    changenone = true, changeany = false;
  boolean doubleTap = false, doubleDoubleTap = false;
  boolean longdouble = false;
  boolean shortrange = false;

  /****************************************************************************
   * State determination
   */

  // Short range detected
  if (range_cm < 50) shortrange = true;

  // Sensor 1 pushed
  if (touch_sensor.touched(CAP_SENSOR_1)) cap1 = true;
  if (touch_sensor.changed(CAP_SENSOR_1)) change1 = true;

  // Sensor 2 pushed
  if (touch_sensor.touched(CAP_SENSOR_2)) cap2 = true;
  if (touch_sensor.changed(CAP_SENSOR_2)) change2 = true;

  // Both pushed
  if (cap1 && cap2) capboth = true;
  if (change1 && change2) changeboth = true;

  // None pushed
  if (!cap1 && !cap2) capnone = true;
  if (!change1 && !change2) changenone = true;

  if (change1 || change2) changeany = true;

  static unsigned long doubleTime = 0;

  // Both became touched (could be one then the other)
  // XXX: One-then-other required as the sense time is fine enough that
  //      it often is triggered that way when trying to touch both.  This
  //      could be improved by tracking the time for the individual sensors
  //      and setting double-tap only if the delay is very short.
  if (capboth && changeany) {
    static unsigned long taptime = 0;

    doubleTap = true;
    
    if (now - taptime < 500) {
      // Rapid double tap
      doubleDoubleTap = true;
      DEBUG_VALUELN(DEBUG_HIGH, "Double tap ms:", now - taptime);
    }

    doubleTime = now;
    taptime = now;
  }


  if (capboth) {
    if ((doubleTime > 0) && ((now - doubleTime) > 750)) {
      // Long touch period
      DEBUG_PRINTLN(DEBUG_HIGH, "Long double touch");
      longdouble = true;
      doubleTime = 0;
    }
  }

  // Neither sensor is touched
  if (capnone) {
    doubleTime = 0;
  }

  /****************************************************************************
   * Handling
   */

  if (longdouble) {
    uiMode = (uiMode + 1) % NUM_UI_MODES;
  }
  

  switch (uiMode) {
  case 0: {
    // If just sensor 1 is being touched
    if (cap1 && !capboth) {
      // Set followup color
      static byte color = 0;
      color++;
      followupConfig.fgColor = pixel_wheel(color);
    }

    // If just sensor 2 is being touched
    if (cap2 && !capboth) {
      static byte color = 0;
      color++;
      modeConfig.fgColor = pixel_wheel(color);
    }

    if (doubleTap) {
      modeConfig.fgColor = pixel_color(255, 255, 255);
    }

    if (doubleDoubleTap) {
      increment_mode();
    }

    if (shortrange) {
      set_followup_mode(MODE_LIGHT_CENTER);
    } else {
      if (get_current_followup() == MODE_LIGHT_CENTER) restore_followup();
    }
    break;
  }
  case 1: {
    if (longdouble) {
      // Just entered mode changing state
      modeConfig.fgColor = pixel_color(255, 255, 255);
      followupConfig.fgColor = pixel_color(0, 0, 255);
      followupConfig.data =
	FACE_LED_MASK(0xFF, ((1 << 0) | (1 << 2) | (1 << 6) | (1 << 8)));
      set_followup_mode(MODE_BLINK_PATTERN);
      DEBUG_PRINTLN(DEBUG_HIGH, "Entered mode change");
    }

    if (cap1 && change1 && !capboth) {
      increment_mode();
    }

    if (cap2 && change2 && !capboth) {
      followupConfig.fgColor = pixel_color(255, 0, 0);
      increment_followup();
    }
    
    break;
  }
  }

  if (cap1 || cap2) {
    /* A sensor is touched, send update to remotes */
    static unsigned long next_send = millis();
    if (now >= next_send) {
      int command = (cap1 << CAP_SENSOR_1) | (cap2 << CAP_SENSOR_2);
      sendInt(command);
      next_send = now + 100;
    }
  } else if (change1 || change2) {
    /* Touch was removed */
    sendInt(0);
  }

#endif
}
