/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2016
 *
 * Sensor code for Triangle Lights
 ******************************************************************************/

#include <Arduino.h>

//#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleLights.h"
#include "Peripherals.h"

void button_interrupt(void);

void initializePins() {
  /* Configure the mode toggle switch */
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(PUSH_BUTTON_INTERRUPT, button_interrupt, CHANGE);

  /* Enable the knob's potentiometer pin's pullup resister */
  digitalWrite(KNOB_PIN, HIGH);
}

/* ***** Momentary switch button **********************************************/

/*
 * This code handles a momentary push button by incrementing a counter
 * every first transition from LOW to HIGH within a fixed time period
 */
#define MIN_DEPRESSED_TIME 500 // In milliseconds
volatile byte buttonValue = 0;
void button_interrupt(void)
{
  // WARNING: Its unsafe to put print statements in an interrupt handler
  static unsigned long prevTime = 0;
  static byte prevValue = LOW;

  unsigned long now = millis();
  byte value = (byte)digitalRead(PUSH_BUTTON_PIN);

  /* Provide a debounce to only change on the first interrupt */
  if ((value == HIGH) && (prevValue == LOW) &&
          (now - prevTime > MIN_DEPRESSED_TIME)) {
    buttonValue++;
    prevTime = now;
  }

  prevValue = value;
}

byte get_button_value() {
  return buttonValue;
}

/* ***** Potentiometer *********************************************************/

uint16_t pot_value = 1024;

void check_pot_value() {
  uint16_t value = analogRead(KNOB_PIN);

  if (abs(value - pot_value) >= 5) {
    pot_value = value;
    DEBUG4_VALUELN("Pot:", pot_value);
  }
}

uint16_t get_pot_value() {
  return pot_value;
}

byte get_pot_byte() {
  return (byte)map(pot_value, 15, 750, 0, 255);;
}

/* ***** Check all sensors for updates *****************************************/

void update_sensors() {
  static unsigned long next_sense = millis();
  unsigned long now = millis();

  if (now > next_sense) {
    next_sense = now + SENSOR_DELAY_MS;

    check_pot_value();
  }
}
