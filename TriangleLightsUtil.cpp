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
    //    DEBUG_VALUELN(DEBUG_HIGH, F(" Photo:"), photo_value);
  }
}

/*******************************************************************************
 * Triangle light patterns
 */

void setAllTriangles(Triangle *triangles, int size, uint32_t color) {
  for (int tri = 0; tri < size; tri++) {
    triangles[tri].setColor(color);
    triangles[tri].mark = 0;
  }
}

void clearTriangles(Triangle *triangles, int size) {
  setAllTriangles(triangles, size, 0);
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

void wheelTriangles(Triangle *triangles, int size) {
  for (int tri = 0; tri < size; tri++) {
    triangles[tri].setColor(pixel_wheel(map(tri, 0, size - 1, 0, 255)));
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

void incrementMarkAll(Triangle *triangles, int size, char incr) {
  for (int tri = 0; tri < size; tri++) {
    int value = triangles[tri].mark + incr;
    if (value < 0) triangles[tri].mark = 0;
    else if (value > 255) triangles[tri].mark = 255;
    else triangles[tri].mark = value;
  }
}

/* Adjusted every led by the indicated amount */
void incrementAll(Triangle *triangles, int size,
                  char r, char g, char b) {
  for (int tri = 0; tri < size; tri++) {
      for (byte c = 0; c < 3; c++) {
	int red = triangles[tri].leds[c].red();
	int green = triangles[tri].leds[c].green();
	int blue = triangles[tri].leds[c].blue();

	red = red + r;
	if (red < 0) red = 0;
	else if (red > 255) red = 255;

	green = green + g;
	if (green < 0) green = 0;
	else if (green > 255) green = 255;

	blue = blue + b;
	if (blue < 0) blue = 0;
	else if (blue > 255) blue = 255;

	triangles[tri].setColor(c, (byte)red, (byte)green, (byte)blue);
      }
  }
}

/******************************************************************************
 * Triangle Patterns
 */

static unsigned long next_time = 0;

/* This iterates through the triangles, lighting the ones with leds */
void trianglesTestPattern(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
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
			     boolean init, pattern_args_t *arg) {
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
			     boolean init, pattern_args_t *arg) {
  static int current = 0;

  if (init) {
    current = 0;
    next_time = millis();
    wheelTriangles(triangles, size);
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
    DEBUG_VALUE(DEBUG_TRACE, F("curr color:"), currentColor);
    DEBUG_VALUELN(DEBUG_TRACE, F("edge color:"), edgeColor);

    triangles[current].setColor(edgeColor);
    triangles[current].edges[edge]->setColor(currentColor);
 }
}

void trianglesLifePattern(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
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
      DEBUG_VALUE(DEBUG_TRACE, F(" "), set[tri]);
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
    DEBUG_PRINTLN(DEBUG_TRACE, F(""));
  }
}

void trianglesLifePattern2(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
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
			     boolean init, pattern_args_t *arg) {
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

    DEBUG_VALUE(DEBUG_TRACE, F("next="), next->id);
    DEBUG_VALUELN(DEBUG_TRACE, F(" vert="), vertex);

    next->setColor(vertex, 255, 0, next->getGreen(vertex));
    current = next;
  }
}

void trianglesCircleCorner2(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
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

    // Set the current led to ever increasing white
    if (current->mark < 250) current->mark += 10;
    current->setColor(vertex, current->mark, current->mark, current->mark);

    if (random(0, 100) < 95){
      // Shift to the left triangle
      next = current->leftOfVertex(vertex);
      if (next->hasLeds) {
	// Find the local vertex adjacent to the one on the previous triangle
	vertex = next->matchVertexRight(current, vertex);
      } else {
	next = current;
      }
    } else {
      next = current;
    }

    if (next == current) {
      // Shift around the current triangle
      vertex = (vertex + 1) % 3;
    }

    incrementAll(triangles, size, -1, -1, -1);
    incrementMarkAll(triangles, size, -1);
    
    next->setColor(vertex, 255, 0, 0);
    current = next;
  }
}

/* Go in a large circle around a pentagon */
void movementCircleRight(Triangle *currentTriangle, byte vertex,
		    Triangle **nextTriangle, byte *nextVertex) {
  static byte phase = 0;

  if (phase % 2 == 0) {
    *nextTriangle = currentTriangle;
    *nextVertex = (vertex + 1) % TRIANGLE_NUM_VERTICES;
  } else {
    *nextTriangle = currentTriangle->rightOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexLeft(currentTriangle, vertex);
  }
  DEBUG_VALUE(DEBUG_TRACE, "phase=", phase);
  DEBUG_VALUE(DEBUG_TRACE, " current=", currentTriangle->id);
  DEBUG_VALUE(DEBUG_TRACE, ",", vertex);
  DEBUG_VALUE(DEBUG_TRACE, " next=", (*nextTriangle)->id);
  DEBUG_VALUELN(DEBUG_TRACE, ",", *nextVertex);

  phase++;
}

/* Go in a large circle around a pentagon */
void movementCircleLeft(Triangle *currentTriangle, byte vertex,
		    Triangle **nextTriangle, byte *nextVertex) {
  static byte phase = 0;

  if (phase % 2 == 0) {
    *nextTriangle = currentTriangle;
    *nextVertex = (vertex + TRIANGLE_NUM_VERTICES - 1) % TRIANGLE_NUM_VERTICES;
  } else {
    *nextTriangle = currentTriangle->leftOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexRight(currentTriangle, vertex);
  }
  phase++;
}

void trianglesCircle(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
  static Triangle *current = &triangles[0];
  static byte vertex = 0;
  static boolean right = true;

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

    // Set the current led to ever increasing white
    if (current->mark < 250) current->mark += 10;
    current->setColor(vertex, current->mark, current->mark, current->mark);

    if (random(0, 100) < 95){
      if (right) {
	movementCircleRight(current, vertex, &next, &vertex);
      } else {
	movementCircleLeft(current, vertex, &next, &vertex);
      }
    } else {
      next = current;
      vertex = VERTEX_CW(vertex);
      right = !right;
    }

    incrementAll(triangles, size, -1, -1, -1);
    incrementMarkAll(triangles, size, -1);

    next->setColor(vertex, 255, 0, 0);
    current = next;
  }
}

void trianglesBuildup(Triangle *triangles, int size, int periodms,
		      boolean init, pattern_args_t *arg) {
  static Triangle *current = &triangles[0];

  if (init) {
    current = &triangles[0];
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Get the color of the current triangle */
    byte color = current->getRed();

    if (color > (255 - 64)) {
      /*
       * If the current color is above the threshold value then
       * increment its mark and choose the next triangle.
       */
      if (current->mark == 0) current->mark = 1;
      else current->mark += current->mark;

      color = 0;

      /* Choose the next triangle */
      Triangle *next = NULL;
      byte edge;
      do {
	edge = random(0, 3);
      } while (!current->edges[edge]->hasLeds);
      next = current->edges[edge];

      /* Set the current triangle's color to its mark value */
      current->setColor(current->mark, current->mark, current->mark);
      current = next;
    }

    /* Increment the color of the current triangle */
    color += 16;
    current->setColor(color, 0, 0);
  }
}

/* This iterates through the triangles, lighting the ones with leds */
void trianglesStaticNoise(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  if (init) {
    next_time = millis();
    setAllTriangles(triangles, size, arg->bgColor);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Set the leds randomly to on off in white */
    for (int tri = 0; tri < size; tri++) {
      for (byte led = 0; led < 3; led++) {
	if (random(0, 100) < 60) {
	  triangles[tri].setColor(led, arg->bgColor);
	} else {
	  int facter = random(0, 7);
	  byte red = pixel_red(arg->fgColor) >> facter;
	  byte green = pixel_green(arg->fgColor) >> facter;
	  byte blue = pixel_blue(arg->fgColor) >> facter;
	  //DEBUG_VALUE(DEBUG_HIGH, "r=", red);
	  //DEBUG_VALUE(DEBUG_HIGH, " g=", green);
	  //DEBUG_VALUELN(DEBUG_HIGH, " b=", blue);
	  triangles[tri].setColor(led, red, green, blue);
	}
      }
    }
  }
}


/* Run a snake randomly around the light */
#define SNAKE_LENGTH 7
void trianglesSnake(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  pattern_args_t *config = (pattern_args_t *)arg;
  static byte snakeTriangles[SNAKE_LENGTH];
  static byte snakeVertices[SNAKE_LENGTH];
  uint32_t values[SNAKE_LENGTH] = {
    255, 128, 64, 32, 16, 8, 4//, 2, 1
  };
  static byte currentIndex = (byte)-1;
  static byte colorMode = 0;

  if (init || (currentIndex == (byte)-1)) {
    DEBUG_PRINT(DEBUG_HIGH, F("Initializing:"));
    next_time = millis();
    setAllTriangles(triangles, size, config->bgColor);
    for (int i = 0; i < SNAKE_LENGTH; i++) {
      snakeTriangles[i] = (byte)-1;
      snakeVertices[i] = (byte)-1;
    }

    /* Start from a random triangle */
    byte tri;
    do {
      tri = random(0, size);
    } while (!triangles[tri].hasLeds);
    currentIndex = 0;
    snakeTriangles[currentIndex] = tri;
    snakeVertices[currentIndex] = random(0, TRIANGLE_NUM_EDGES);

    colorMode++;
    DEBUG_VALUE(DEBUG_HIGH, " colormode=", colorMode);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Determine which colors to use for the snake */
    switch (colorMode % 5) {
    case 0: {
      for (int i = 0; i < SNAKE_LENGTH; i++) {
	values[i] = pixel_wheel(map(i, 0, SNAKE_LENGTH - 1, 0, 255));
      }
      break;
    }
    case 1: {
      for (int i = 0; i < SNAKE_LENGTH; i++) {
	byte red = 255 >> i;
	values[i] = pixel_color(red, 0, 0);
      }
      break;
    }
    case 2: {
      for (int i = 0; i < SNAKE_LENGTH; i++) {
	byte green = 255 >> i;
	values[i] = pixel_color(0, green, 0);
      }
      break;
    }
    case 3: {
      for (int i = 0; i < SNAKE_LENGTH; i++) {
	byte blue = 255 >> i;
	values[i] = pixel_color(0, 0, blue);
      }
      break;
    }
    case 4: {
      for (int i = 0; i < SNAKE_LENGTH; i++) {
	byte color = 255 >> i;
	values[i] = pixel_color(color, color, color);
      }
      break;
    }
    }

    /* Clear the tail */
    byte tri, vert;
    byte nextIndex;

    /* Choose the next location */
    Triangle *current = &triangles[snakeTriangles[currentIndex]];
    tri = (byte)-1;
    boolean found = false;

    byte startDirection = random(0, 4);
    for (byte direction = 0; direction < 4; direction++) {
      switch ((startDirection + direction) % 4) {
      case 0:
      default:
	// Triangle to the left
	tri = current->leftOfVertex(snakeVertices[currentIndex])->id;
	vert = triangles[tri].matchVertexRight(current, 
					       snakeVertices[currentIndex]);	
	break;
      case 1:
	// Triangle to the right
	tri = current->rightOfVertex(snakeVertices[currentIndex])->id;
	vert = triangles[tri].matchVertexLeft(current, 
					       snakeVertices[currentIndex]);
	break;
      case 2:
	// Same triangle, vertex to the left
	vert = (snakeVertices[currentIndex] + TRIANGLE_NUM_EDGES - 1) % TRIANGLE_NUM_EDGES;
	tri = snakeTriangles[currentIndex];
	break;
      case 3: {
	// Same triangle, vertex to the right
	vert = (snakeVertices[currentIndex] + 1) % TRIANGLE_NUM_EDGES;
	tri = snakeTriangles[currentIndex];
	break;
      }
      }

      if ((triangles[tri].leds[vert].color() == config->bgColor) &&
	  (triangles[tri].hasLeds)) {
	// Verify that the choosen vertex is dark
	found = true;
	break;
      }
    }
    if (!found) {
      currentIndex = (byte)-1;
      DEBUG_PRINTLN(DEBUG_HIGH, F("End of snake"));
      return;
    }

    if (currentIndex == 0) {
      nextIndex = SNAKE_LENGTH - 1;
    } else {
      nextIndex = currentIndex - 1;
    }
    if ((snakeTriangles[nextIndex] < size) && 
	(snakeVertices[nextIndex] < TRIANGLE_NUM_VERTICES)) 
      triangles[snakeTriangles[nextIndex]].setColor(snakeVertices[nextIndex], 
						    config->bgColor);

    /* Move the array index*/
    currentIndex = nextIndex;
    snakeTriangles[currentIndex] = tri;
    snakeVertices[currentIndex] = vert;

    /* Set the led values */
    for (byte i = 0; i < SNAKE_LENGTH; i++) {
      byte valueIndex = (i + SNAKE_LENGTH - currentIndex) % SNAKE_LENGTH;
      if (snakeTriangles[i] != (byte)-1) {
	
        triangles[snakeTriangles[i]].setColor(snakeVertices[i],
                                              values[valueIndex]);
      }
    }

    DEBUG_VALUE(DEBUG_TRACE, F(" i:"), currentIndex);
    DEBUG_VALUE(DEBUG_TRACE, F(" tri:"), tri);
    DEBUG_VALUELN(DEBUG_TRACE, F(" vert:"), vert);
  }
}

/* Just set all triangles to the indicate foreground color */
void trianglesSetAll(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  if (init) {
    next_time = millis();
    setAllTriangles(triangles, size, arg->fgColor);
  }

  if (millis() > next_time) {
    next_time += periodms;
    DEBUG_HEXVAL(DEBUG_HIGH, "color=", arg->fgColor);
    DEBUG_HEXVAL(DEBUG_HIGH, " getColor=", triangles[0].getColor());
    DEBUG_HEXVAL(DEBUG_HIGH, " getRed=", triangles[0].getRed());
    DEBUG_HEXVAL(DEBUG_HIGH, " getGreen=", triangles[0].getGreen());
    DEBUG_HEXVALLN(DEBUG_HIGH, " getBlue=", triangles[0].getBlue());
  }
}
