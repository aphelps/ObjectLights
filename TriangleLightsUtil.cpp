#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleLights.h"

void initializePins() {
  /* Configure the mode toggle switch */
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(0, buttonInterrupt, CHANGE);

  /* Turn on input pullup on analog light sensor pin */
  digitalWrite(PHOTO_PIN, HIGH);
}

volatile uint16_t buttonValue = 0;
void buttonInterrupt(void)
{
  static unsigned long prevTime = 0;
  static int prevValue = LOW;
  long now = millis();
  int value = digitalRead(PUSH_BUTTON_PIN);

  /* Provide a debounce to only change on the first interrupt */
  if ((value == HIGH) && (prevValue == LOW) && (now - prevTime > 500)) {
    buttonValue++;
    prevTime = now;

    DEBUG_VALUE(DEBUG_HIGH, "value=", value);
    DEBUG_VALUE(DEBUG_HIGH, " buttonValue=", buttonValue);
    DEBUG_VALUE(DEBUG_HIGH, " prevTime=", prevTime);
  }

  prevValue = value;
}

int getButtonValue() {
  return buttonValue;
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
      photo_dark = true;
    }
    DEBUG_VALUE(DEBUG_HIGH, " Photo:", photo_value);
  }
}

/*******************************************************************************
 * Triangle light patterns
 */

/* This iterates through the triangles, lighting the ones with leds */
void trianglesTestPattern(Triangle **triangles, int size, int periodms) {
  static unsigned long next_time = millis();
  static int current = 0;

  if (millis() > next_time) {
    next_time += periodms;

    /* Clear the color of the previous triangle */
    triangles[current % size]->setColor(0, 0, 0);

    current++;

    /* Set the color on the new triangle */
    triangles[current % size]->setColor(255, 0, 0);
  }
}

void trianglesRandomNeighbor(Triangle **triangles, int size, int periodms) {
  static unsigned long next_time = millis();
  static int current = 0;

  if (millis() > next_time) {
    next_time += periodms;

    /* Clear the color of the previous triangle */
    triangles[current % size]->setColor(0, 0, 0);

    /* Choose the next triangle */
    boolean has_next = false;
    while (!has_next) {
      //XXX
    }

    /* Set the color on the new triangle */
    triangles[current % size]->setColor(255, 0, 0);
  }

}
