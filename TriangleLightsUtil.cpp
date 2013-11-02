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

void randomTriangles(Triangle *triangles, int size) {
  for (int tri = 0; tri < size; tri++) {
    byte red = random(0, 16);
    byte green = random(0, 16);
    byte blue = random(0, 16);
    red = red * red;
    green = green * green;
    blue = blue * blue;

    triangles[tri].setColor(red, green, blue);
  }
}

void binaryTriangles(Triangle *triangles, int size, uint32_t color, int thresh)
{
  for (int tri = 0; tri < size; tri++) {
    boolean set = (random(0, 100) > thresh);
    if (set) triangles[tri].setColor(color);
    else triangles[tri].setColor(0);
  }
}

void randomBinaryTriangles(Triangle *triangles, int size, byte color, int thresh)
{
  for (int tri = 0; tri < size; tri++) {
    boolean red = (random(0, 100) > thresh);
    boolean green = (random(0, 100) > thresh);
    boolean blue = (random(0, 100) > thresh);

    triangles[tri].setColor((red ? color : 0),
			    (blue ? color : 0),
			    (green ? color : 0));
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

    current = (current + 1) % size;

    /* Set the color on the new triangle and its edges */
    triangles[current % size].setColor(255, 0, 0);
    triangles[current % size].edges[0]->setColor(8, 00, 00);
    triangles[current % size].edges[1]->setColor(8, 00, 00);
    triangles[current % size].edges[2]->setColor(8, 00, 00);
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
    for (byte edge = 0; edge < 3; edge++) {
      current->edges[edge]->setColor(0, 0, 0);
    }

    /* Choose the next triangle */
    byte edge;
    do {
      edge = random(0, 2);
    } while (!current->edges[edge]->hasLeds);
    current = current->edges[edge];

    /* Set the color on the new triangle */
    current->setColor(0, 0, 255);
    for (byte edge = 0; edge < 3; edge++) {
      current->edges[edge]->setColor(0, 0, 8);
    }
  }
}

/*
 * Triangles are initialized to a random color, each cycle one swaps colors
 * with a random neighbor.
 */
void trianglesSwapPattern(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();
  static int current = 0;

  if (init) {
    current = 0;
    next_time = millis();
    randomTriangles(triangles, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

    do {
      current = random(0, size);
    } while (!triangles[current].hasLeds);

    int edge;
    do {
      edge = random(0, 3);
    } while (!triangles[current].edges[edge]->hasLeds);

    uint32_t currentColor = triangles[current].getColor();
    uint32_t edgeColor = triangles[current].edges[edge]->getColor();
    DEBUG_VALUE(DEBUG_HIGH, "curr color:", currentColor);
    DEBUG_VALUELN(DEBUG_HIGH, "edge color:", edgeColor);

    triangles[current].setColor(edgeColor);
    triangles[current].edges[edge]->setColor(currentColor);
 }
}

void trianglesLifePattern(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();
  static int current = 0;

  if (init) {
    current = 0;
    next_time = millis();
    binaryTriangles(triangles, size, pixel_color(255, 0, 0), 25);
  }

  if (millis() > next_time) {
    next_time += periodms;

    int set[size];
    for (int tri = 0; tri < size; tri++) {
      set[tri] = 0;
      if (triangles[tri].getColor() != 0) set[tri]++;
      for (byte edge = 0; edge < 3; edge++) {
	if (triangles[tri].edges[edge]->getColor() != 0) set[tri]++;
      }
    }

    for (int tri = 0; tri < size; tri++) {
      DEBUG_VALUE(DEBUG_HIGH, " ", set[tri]);
      switch (set[tri]) {
      case 0:
      case 3:
      case 4:
	triangles[tri].setColor(0);
	break;
      case 1:
      case 2:
	triangles[tri].setColor(255, 0, 0);
	break;
      }
    }
    DEBUG_PRINTLN(DEBUG_HIGH, "");
  }
}

void trianglesLifePattern2(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();
  static unsigned long next_reset = millis();

  if (init || (millis() > next_reset)) {
    next_reset = millis() + 60000;
    next_time = millis();
    randomBinaryTriangles(triangles, size, 255, 25);
  }

  if (millis() > next_time) {
    next_time += periodms;

    int set[size][3];
    for (int tri = 0; tri < size; tri++) {
      if (triangles[tri].getRed() != 0) set[tri][0] = 1; else set[tri][0] = 0;
      if (triangles[tri].getGreen() != 0) set[tri][1] = 1; else set[tri][1] = 0;
      if (triangles[tri].getBlue() != 0) set[tri][2] = 1; else set[tri][2] = 0;
      for (byte edge = 0; edge < 3; edge++) {
	if (triangles[tri].edges[edge]->getRed() != 0) set[tri][0]++;
	if (triangles[tri].edges[edge]->getGreen() != 0) set[tri][1]++;
	if (triangles[tri].edges[edge]->getBlue() != 0) set[tri][2]++;
      }
    }

    for (int tri = 0; tri < size; tri++) {
      byte color[3];
      for (byte c = 0; c < 3; c++) {
	switch (set[tri][c]) {
	case 0:
	case 3:
	case 4:
	  color[c] = 0;
	  break;
	case 1:
	case 2:
	  color[c] = 255;
	  break;
	}
      }

      triangles[tri].setColor(color[0], color[1], color[2]);
    }
  }
}

void trianglesCircleCorner(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();

  static Triangle *current = &triangles[0];
  static int vertex = 0;

  if (init) {
    current = &triangles[0];
    vertex = 0;
    next_time = millis();
    clearTriangles(triangles, size);
    current->setColor(vertex, 255, 0, 0);
  }

  if (millis() > next_time) {
    Triangle *next;
    next_time += periodms;

    byte storedRed = current->getBlue(vertex) + 5;
    if (random(0, 200) + storedRed > 220) storedRed = 0;
    current->setColor(vertex, storedRed, storedRed, storedRed);

    if (random(0, 100) < 95) {
      next = current->vertices[vertex][0];
      if (next->hasLeds) {
	vertex = next->matchVertex(current);
	next->setColor(vertex, 0, next->getRed(vertex), 0);
      } else {
	next = current;
      }
    } else {
      next = current;
    }

    if (next == current) {
      next = current;
      vertex = (vertex + 1) % 3;
      next->setColor(vertex, 0, next->getRed(vertex), 0);
    }

    DEBUG_VALUE(DEBUG_HIGH, "next=", next->id);
    DEBUG_VALUELN(DEBUG_HIGH, " vert=", vertex);

    next->setColor(vertex, 255, 0, next->getGreen(vertex));
    current = next;
  }
}

void trianglesCircleCorner2(Triangle *triangles, int size, int periodms,
			     boolean init) {
  static unsigned long next_time = millis();

  static Triangle *current = &triangles[0];
  static int vertex = 0;

  if (init) {
    current = &triangles[0];
    vertex = 0;
    next_time = millis();
    clearTriangles(triangles, size);
    current->setColor(vertex, 255, 0, 0);
  }

  if (millis() > next_time) {
    Triangle *next;
    next_time += periodms;

    byte storedRed = current->getBlue(vertex) + 5;
    if (random(0, 200) + storedRed > 220) storedRed = 0;
    current->setColor(vertex, storedRed, storedRed, storedRed);

    if (random(0, 100) < 95) {
      next = current->leftOfVertex(vertex); // Shift to the left triangle
      if (next->hasLeds) {
	vertex = next->matchVertexRight(current, vertex); // Find the local vertex adjacent to the one on the previous triangle
	next->setColor(vertex, 0, next->getRed(vertex), 0);
      } else {
	next = current;
      }
    } else {
      next = current;
    }

    if (next == current) {
      next = current;
      vertex = (vertex + 1) % 3;
      next->setColor(vertex, 0, next->getRed(vertex), 0);
    }

    DEBUG_VALUE(DEBUG_HIGH, "next=", next->id);
    DEBUG_VALUELN(DEBUG_HIGH, " vert=", vertex);

    next->setColor(vertex, 255, 0, next->getGreen(vertex));
    current = next;
  }
}

void trianglesBuildup(Triangle *triangles, int size, int periodms,
		      boolean init, uint32_t fgColor, uint32_t bgColor) {
  static unsigned long next_time = millis();

  static Triangle *current = &triangles[0];
  static Triangle *next = NULL;

  if (init) {
    current = &triangles[0];
    next = current;
    next->mark = 1;
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Clear the color of the previous triangle */
    current->setColor(current->mark, current->mark, current->mark);

    /* Set the color on the new triangle */
    static byte color = 64;
    next->setColor(color, 0, 0);
    if (color > (255 - 64)) {
      next->mark += next->mark;
      if (next->mark == 0) next->mark = 1;
      current = next;
      color = 0;

      /* Choose the next triangle */
      byte edge;
      do {
	edge = random(0, 3);
      } while (!current->edges[edge]->hasLeds);
      next = current->edges[edge];

    } else {
      color += 64;
    }
  }
}

/* This iterates through the triangles, lighting the ones with leds */
void trianglesStaticNoise(Triangle *triangles, int size, int periodms,
			  boolean init) {
  static unsigned long next_time = millis();

  if (init) {
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Set the leds randomly to on off in white */
    for (int tri = 0; tri < size; tri++) {
      for (byte led = 0; led < 3; led++) {
	if (random(0, 100) < 60) {
	  triangles[tri].setColor(led, 0);
	} else {
	  int value = random(1,17);
	  value = (value * value) - 1;
	  triangles[tri].setColor(led, value, value, value);
	}
      }
    }
  }
}
