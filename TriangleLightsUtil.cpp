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
    // WARNING: Its unsafe to put print statements in an interrupt handler
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
    //    DEBUG_VALUE(DEBUG_HIGH, " Photo:", photo_value);
  }
}

/*******************************************************************************
 * Triangle light patterns
 */

void clearTriangles(Triangle *triangles, int size) {
  for (int tri = 0; tri < size; tri++) {
    triangles[tri].setColor(0, 0, 0);
  }
}

/* This iterates through the triangles, lighting the ones with leds */
void trianglesTestPattern(Triangle *triangles, int size, int periodms,
			  boolean init) {
  static unsigned long next_time = millis();
  static int current = 0;

  if (init) {
    current = 0;
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Clear the color of the previous triangle and its edges*/
    triangles[current % size].setColor(0, 0, 0);
    for (byte edge = 0; edge < 3; edge++) {
      triangles[current % size].edges[edge]->setColor(0, 0, 0);
    }

    //    current = (current + 1) % size;

    /* Set the color on the new triangle and its edges */
    triangles[current % size].setColor(0, 255, 0);
    triangles[current % size].edges[0]->setColor(32, 32, 00); // yellow
    triangles[current % size].edges[1]->setColor(32, 00, 32); // purple
    triangles[current % size].edges[2]->setColor(00, 32, 32); // teal
  }
}

/*
 * This pattern lights a single triangle, moving randomly between neighboring
 * triangles.
 */
void trianglesRandomNeighbor(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();
  static Triangle *current = &triangles[0];

  if (init) {
    current = &triangles[0];
    next_time = millis();
    clearTriangles(triangles, size);
}

  if (millis() > next_time) {
    next_time += periodms;

    /* Clear the color of the previous triangle */
    current->setColor(0, 0, 0);

    /* Choose the next triangle */
    byte edge;
    do {
      edge = random(0, 2);
    } while (!current->edges[edge]->hasLeds);
    current = current->edges[edge];

    /* Set the color on the new triangle */
    current->setColor(0, 0, 255);
  }
}

/*
 * Triangles are initialized to a random color, each cycle they swap colors
 * with a random neighbor.
 */
void trianglesSwapPattern(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();
  static int current = 0;

  if (init) {
    current = 0;
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Clear the color of the previous triangle */
    triangles[current % size].setColor(0, 0, 0);

    /* Choose the next triangle */
    do {
      current = random(0, 2);
    } while (!triangles[current].hasLeds);

    /* Set the color on the new triangle */
    // XXX - triangles[current % size].setColor(255, 0, 0);
  }
}
