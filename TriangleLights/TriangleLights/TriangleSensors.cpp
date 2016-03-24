/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2015
 *
 * Sensor code for Triangle Lights
 ******************************************************************************/

#include <Arduino.h>

//#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "../../Libraries/TriangleLibrary/TriangleLights.h"

void button_interrupt(void);

void initializePins() {
  /* Configure the mode toggle switch */
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(PUSH_BUTTON_INTERRUPT, button_interrupt, CHANGE);

  /* Turn on input pullup on analog light sensor pin */
  digitalWrite(PHOTO_PIN, HIGH);

  digitalWrite(POT_PIN, HIGH);
}

volatile uint16_t buttonValue = 0;
void button_interrupt(void) 
{
  static unsigned long prevTime = 0;
  static int prevValue = LOW;
  long now = millis();
  int value = digitalRead(PUSH_BUTTON_PIN);

  /* Provide a debounce to only change on the first interrupt */
  if ((value == HIGH) && (prevValue == LOW) && (now - prevTime > 500)) {
    buttonValue++;
    prevTime = now;
    // WARNING: Its unsafe to put print statements in an interrupt handler
  }

  prevValue = value;
}

byte get_button_value() {
  return buttonValue;
}


/* ***** Photo sensor *********************************************************/

uint16_t photo_value = 1024;
boolean photo_dark = false;
void check_photo_value(void)
{
  photo_value = analogRead(PHOTO_PIN);
  if ((photo_value > PHOTO_THRESHOLD_HIGH) && (!photo_dark)) {
    DEBUG4_VALUELN(" Photo dark:", photo_value);
    photo_dark = true;
  } else if ((photo_value < PHOTO_THRESHOLD_LOW) && (photo_dark)) {
    photo_dark = false;
    DEBUG4_VALUELN(" Photo light:", photo_value);
  }
}

uint16_t get_photo_value() {
  return photo_value;
}

boolean is_dark() {
  return photo_dark;
}

/* ***** Potentiometer *********************************************************/

uint16_t pot_value = 1024;

void check_pot_value() {
  pot_value = analogRead(POT_PIN);
}

uint16_t get_pot_value() {
  return pot_value;
}

/* ***** Check all sensors for updates *****************************************/

void update_sensors() {
  static unsigned long next_sense = millis();
  unsigned long now = millis();

  if (now > next_sense) {
    next_sense = now + SENSOR_DELAY_MS;

    check_pot_value();

    check_photo_value();
  }
}
