#include <Arduino.h>

//#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleLights.h"

void initializePins() {
  /* Configure the mode toggle switch */
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(PUSH_BUTTON_INTERRUPT, buttonInterrupt, CHANGE);

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
    if ((photo_value > PHOTO_THRESHOLD_HIGH) && (!photo_dark)) {
      DEBUG4_VALUELN(" Photo dark:", photo_value);
      photo_dark = true;
    } else if ((photo_value < PHOTO_THRESHOLD_LOW) && (photo_dark)) {
      photo_dark = false;
      DEBUG4_VALUELN(" Photo light:", photo_value);
    }
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
	int red = triangles[tri].leds[c].red;
	int green = triangles[tri].leds[c].green;
	int blue = triangles[tri].leds[c].blue;

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
 * Color modes
 */

/* White builds up as an vertex is visited and fades over time */
void colorWhiteBuildupFade(Triangle *current, byte currVertex,
			   Triangle *next, byte nextVertex,
			   Triangle *triangles, byte size,
			   byte increment) {
  if (current->mark <= (255 - increment)) current->mark += increment;
  else current->mark = 255;
  current->setColor(currVertex, current->mark, current->mark, current->mark);

  incrementAll(triangles, size, -1, -1, -1);
  incrementMarkAll(triangles, size, -1);

  next->mark = next->getRed(nextVertex);
  next->setColor(nextVertex, 255, 0, 0);
}

/* Pixels are set to a pixel_wheel value and face over time */
void colorRainbowTrail(Triangle *current, byte currVertex,
		       Triangle *next, byte nextVertex,
		       Triangle *triangles, byte size,
		       int increment) {
  static byte wheel_position = 0;

  // Perhaps a shift instead?  How to achieve an evenish fade?
  incrementAll(triangles, size, increment, increment, increment);

  next->setColor(nextVertex, pixel_wheel(wheel_position));
  wheel_position += 5;
}


/******************************************************************************
 * Triangle traversals
 */

void movementCornerCW(Triangle *currentTriangle, byte vertex,
                      Triangle **nextTriangle, byte *nextVertex) {
  *nextTriangle = currentTriangle->leftOfVertex(vertex);
  if (*nextTriangle == NULL) {
    *nextVertex = Triangle::NO_VERTEX;
  } else {
    *nextVertex = (*nextTriangle)->matchVertexRight(currentTriangle, vertex);
  }
}

void movementCornerCCW(Triangle *currentTriangle, byte vertex,
                       Triangle **nextTriangle, byte *nextVertex) {
  *nextTriangle = currentTriangle->rightOfVertex(vertex);
  if (*nextTriangle == NULL) {
    *nextVertex = Triangle::NO_VERTEX;
  } else {
    *nextVertex = (*nextTriangle)->matchVertexLeft(currentTriangle, vertex);
  }
}

/* Go in a large circle around a pentagon */
void movementCircleCW(Triangle *currentTriangle, byte vertex,
		    Triangle **nextTriangle, byte *nextVertex) {
  static byte phase = 0;

  if (phase % 2 == 0) {
    *nextTriangle = currentTriangle;
    *nextVertex = VERTEX_CW(vertex);
  } else {
    *nextTriangle = currentTriangle->rightOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexLeft(currentTriangle, vertex);
  }
  DEBUG5_VALUE("phase=", phase);
  DEBUG5_VALUE(" current=", currentTriangle->id);
  DEBUG5_VALUE(",", vertex);
  DEBUG5_VALUE(" next=", (*nextTriangle)->id);
  DEBUG5_VALUELN(",", *nextVertex);

  phase++;
}

/* Go in a large circle around a pentagon */
void movementCircleCCW(Triangle *currentTriangle, byte vertex,
		    Triangle **nextTriangle, byte *nextVertex) {
  static byte phase = 0;

  if (phase % 2 == 0) {
    *nextTriangle = currentTriangle;
    *nextVertex = VERTEX_CCW(vertex);
  } else {
    *nextTriangle = currentTriangle->leftOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexRight(currentTriangle, vertex);
  }
  phase++;
}

/* Follow a belt around the triangles */
void movementBelt1(Triangle *currentTriangle, byte vertex,
		    Triangle **nextTriangle, byte *nextVertex) {
  static byte phase = 0;

  switch (phase % 6) {
  case 0: case 1: {
    *nextTriangle = currentTriangle;
    *nextVertex = VERTEX_CW(vertex);
    break;
  }
  case 2: {
    *nextTriangle = currentTriangle->rightOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexLeft(currentTriangle, vertex);
    break;
  }
  case 3: case 4: {
    *nextTriangle = currentTriangle;
    *nextVertex = VERTEX_CCW(vertex);
    break;
  }
  case 5: {
    *nextTriangle = currentTriangle->leftOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexRight(currentTriangle, vertex);
    break;
  }
  }
  phase++;
}

/* Follow a belt around the triangles */
void movementBelt2(Triangle *currentTriangle, byte vertex,
		    Triangle **nextTriangle, byte *nextVertex) {
  static byte phase = 0;

  switch (phase % 6) {
  case 3: case 4: {
    *nextTriangle = currentTriangle;
    *nextVertex = VERTEX_CW(vertex);
    break;
  }
  case 5: {
    *nextTriangle = currentTriangle->rightOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexLeft(currentTriangle, vertex);
    break;
  }
  case 0: case 1: {
    *nextTriangle = currentTriangle;
    *nextVertex = VERTEX_CCW(vertex);
    break;
  }
  case 2: {
    *nextTriangle = currentTriangle->leftOfVertex(vertex);
    *nextVertex = (*nextTriangle)->matchVertexRight(currentTriangle, vertex);
    break;
  }
  }
  phase++;
}

/******************************************************************************
 * Global effects
 */
void mergeAdjacent(Triangle *triangles, int size) {
  Triangle *rightT, *leftT;
  byte rightV, leftV;
  uint32_t color;

  for (int tri = 0; tri < size; tri++) {
    for (byte vertex = 0; vertex < Triangle::NUM_VERTICES; vertex++) {
      movementCornerCCW(&triangles[tri], vertex, &rightT, &rightV);
      movementCornerCW(&triangles[tri], vertex, &leftT, &leftV);
      
      uint32_t leftColor, rightColor;
      if (leftT == NULL) leftColor = 0;
      else leftColor = leftT->getColor(leftV);

      if (rightT == NULL) rightColor = 0;
      else rightColor = rightT->getColor(rightV);

      // Get the midpoint of the neighbors
      color = fadeTowards(leftColor, rightColor, 50);
      // Fade towards the midpoint color
      color = fadeTowards(triangles[tri].getColor(vertex), color, 5);

      triangles[tri].setColor(vertex, color);
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
      triangles[current % size].getEdge(edge)->setColor(0, 0, 0);
    }

    current = (current + 1) % size;

    /* Set the color on the new triangle and its edges */
    triangles[current % size].setColor(255, 0, 0);
    triangles[current % size].getEdge(0)->setColor(8, 00, 00);
    triangles[current % size].getEdge(1)->setColor(8, 00, 00);
    triangles[current % size].getEdge(2)->setColor(8, 00, 00);
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
      current->getEdge(edge)->setColor(0, 0, 0);
    }

    /* Choose the next triangle */
    byte edge;
    do {
      edge = random(0, 2);
    } while (!current->getEdge(edge)->hasLeds());
    current = current->getEdge(edge);

    /* Set the color on the new triangle */
    current->setColor(0, 0, 255);
    for (byte edge = 0; edge < 3; edge++) {
      current->getEdge(edge)->setColor(0, 0, 8);
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
    } while (!triangles[current].hasLeds());
    
    int edge;
    do {
      edge = random(0, 3);
    } while (!triangles[current].getEdge(edge)->hasLeds());
 
    uint32_t currentColor = triangles[current].getColor();
    uint32_t edgeColor = triangles[current].getEdge(edge)->getColor();
    DEBUG5_VALUE("curr color:", currentColor);
    DEBUG5_VALUELN("edge color:", edgeColor);

    triangles[current].setColor(edgeColor);
    triangles[current].getEdge(edge)->setColor(currentColor);
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
	if (triangles[tri].getEdge(edge)->getColor() != 0) set[tri]++;
      }
    }

    for (int tri = 0; tri < size; tri++) {
      DEBUG5_VALUE(" ", set[tri]);
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
    DEBUG5_PRINTLN("");
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
	if (triangles[tri].getEdge(edge)->getRed() != 0) set[tri][0]++;
	if (triangles[tri].getEdge(edge)->getGreen() != 0) set[tri][1]++;
	if (triangles[tri].getEdge(edge)->getBlue() != 0) set[tri][2]++;
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
    current = &triangles[random(0, size)];
    vertex = random(0, 3);
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    Triangle *next;
    next_time += periodms;

    byte storedRed = current->getBlue(vertex) + 5;
    if (random(0, 200) + storedRed > 220) storedRed = 0;
    current->setColor(vertex, storedRed, storedRed, storedRed);

    if (random(0, 100) < 95) {
      next = current->getVertex(vertex, 0);
      if (next->hasLeds()) {
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

    DEBUG5_VALUE("next=", next->id);
    DEBUG5_VALUELN(" vert=", vertex);

    next->setColor(vertex, 255, 0, next->getGreen(vertex));
    current = next;
  }
}

void trianglesCircleCorner2(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
  static Triangle *current = &triangles[0];
  static int vertex = 0;

  if (init) {
    current = &triangles[random(0, size)];
    vertex = random(0, 3);
    next_time = millis();
    clearTriangles(triangles, size);
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
      if (next->hasLeds()) {
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

void trianglesCircle(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
  static Triangle *current = &triangles[0];
  static byte vertex = 0;
  static boolean right = true;

  if (init) {
    current = &triangles[random(0, size)];
    vertex = random(0, 3);
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    Triangle *next;
    next_time += periodms;

    // Set the current led to ever increasing white
    if (current->mark < 250) current->mark += 10;
    current->setColor(vertex, current->mark, current->mark, current->mark);

    if (random(0, 100) < 95){
      if (right) {
	movementCircleCW(current, vertex, &next, &vertex);
      } else {
	movementCircleCCW(current, vertex, &next, &vertex);
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

void trianglesLooping(Triangle *triangles, int size, int periodms,
                      boolean init, pattern_args_t *arg) {
  static Triangle *current = &triangles[0];
  static byte vertex = 0;
  static byte mode = 0;

  if (init) {
    current = &triangles[random(0, size)];
    vertex = random(0, 3);
    next_time = millis();
    clearTriangles(triangles, size);
  }

  if (millis() > next_time) {
    Triangle *next = NULL;
    byte nextVertex;
    byte increment = 10;
    byte threshold = 5;
    next_time += periodms;

    while (next == NULL) {
      switch (mode % 6) {
        case 0:
        movementCornerCW(current, vertex, &next, &nextVertex);
        increment = 5;
        threshold = 5;
        break;
        case 1:
        movementCornerCCW(current, vertex, &next, &nextVertex);
        increment = 5;
        threshold = 5;
        break;
        case 2:
        movementCircleCW(current, vertex, &next, &nextVertex);
        increment = 10;
        threshold = 5;
        break;
        case 3:
        movementCircleCCW(current, vertex, &next, &nextVertex);
        increment = 10;
        threshold = 5;
        break;
        case 4:
        movementBelt1(current, vertex, &next, &nextVertex);
        increment = 15;
        threshold = 2;
        break;
        case 5:
        movementBelt2(current, vertex, &next, &nextVertex);
        increment = 15;
        threshold = 2;
        break;
      }

      if (random(0, 100) < threshold) {
        mode += random(0, 6);
      }
    }

    //colorWhiteBuildupFade(current, vertex, next, nextVertex, triangles, size, increment);

    colorRainbowTrail(current, vertex, next, nextVertex, triangles, size,
		      -5);
    // -5);

    current = next;
    vertex = nextVertex;
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
      } while (!current->getEdge(edge)->hasLeds());
      next = current->getEdge(edge);

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
	  //DEBUG4_VALUE("r=", red);
	  //DEBUG4_VALUE(" g=", green);
	  //DEBUG4_VALUELN(" b=", blue);
	  triangles[tri].setColor(led, red, green, blue);
	}
      }
    }
  }
}


/* Run a snake randomly around the light */
#define SNAKE_LENGTH 8
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
    DEBUG4_PRINT("Initializing:");
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
    } while (!triangles[tri].hasLeds());
    currentIndex = 0;
    snakeTriangles[currentIndex] = tri;
    snakeVertices[currentIndex] = random(0, Triangle::NUM_EDGES);

    colorMode++;
    DEBUG4_VALUE(" colormode=", colorMode);
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
    byte currentVertex = snakeVertices[currentIndex];
    tri = (byte)-1;
    boolean found = false;

    byte startDirection = random(0, 4);
    for (byte direction = 0; direction < 4; direction++) {
      switch ((startDirection + direction) % 4) {
      case 0:
      default:
	// Triangle to the left
	tri = current->leftOfVertex(currentVertex)->id;
	vert = triangles[tri].matchVertexRight(current, 
					       currentVertex);	
	break;
      case 1:
	// Triangle to the right
	tri = current->rightOfVertex(currentVertex)->id;
	vert = triangles[tri].matchVertexLeft(current, 
					       currentVertex);
	break;
      case 2:
	// Same triangle, vertex to the left
	vert = (currentVertex + Triangle::NUM_EDGES - 1) % Triangle::NUM_EDGES;
	tri = snakeTriangles[currentIndex];
	break;
      case 3: {
	// Same triangle, vertex to the right
	vert = (currentVertex + 1) % Triangle::NUM_EDGES;
	tri = snakeTriangles[currentIndex];
	break;
      }
      }

      if ((triangles[tri].leds[vert].color() == config->bgColor) &&
	  (triangles[tri].hasLeds())) {
	// Verify that the choosen vertex is dark
	found = true;
	break;
      }
    }
    if (!found) {
      currentIndex = (byte)-1;
      DEBUG4_PRINTLN("End of snake");
      return;
    }

    if (currentIndex == 0) {
      nextIndex = SNAKE_LENGTH - 1;
    } else {
      nextIndex = currentIndex - 1;
    }
    if ((snakeTriangles[nextIndex] < size) && 
	(snakeVertices[nextIndex] < Triangle::NUM_VERTICES)) 
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

    DEBUG5_VALUE(" i:", currentIndex);
    DEBUG5_VALUE(" tri:", tri);
    DEBUG5_VALUELN(" vert:", vert);
  }
}

/* Run a snake randomly around the light */
#define SNAKE2_LENGTH 12
void trianglesSnake2(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  pattern_args_t *config = (pattern_args_t *)arg;
  static byte snakeTriangles[SNAKE2_LENGTH];
  static byte snakeVertices[SNAKE2_LENGTH];
  uint32_t values[SNAKE2_LENGTH] = {
    255, 128, 64, 32, 16, 8, 4//, 2, 1
  };
  static byte currentIndex = (byte)-1;
  static byte colorMode = 0;

  if (init || (currentIndex == (byte)-1)) {
    DEBUG4_PRINT("Initializing:");
    next_time = millis();
    setAllTriangles(triangles, size, config->bgColor);
    for (int i = 0; i < SNAKE2_LENGTH; i++) {
      snakeTriangles[i] = Triangle::NO_ID;
      snakeVertices[i] = Triangle::NO_VERTEX;
    }

    /* Start from a random triangle */
    byte tri;
    do {
      tri = random(0, size);
    } while (!triangles[tri].hasLeds());
    currentIndex = 0;
    snakeTriangles[currentIndex] = tri;
    snakeVertices[currentIndex] = random(0, Triangle::NUM_EDGES);

    colorMode++;
    DEBUG4_VALUE(" colormode=", colorMode);
  }

  if (millis() > next_time) {
    next_time += periodms;

    /* Determine which colors to use for the snake */
    switch (colorMode % 1) {
      case 0: {
        for (int i = 0; i < SNAKE2_LENGTH; i++) {
          values[i] = pixel_wheel(map(i, 0, SNAKE2_LENGTH - 1, 0, 255));
        }
        break;
      }
      case 1: {
        for (int i = 0; i < SNAKE2_LENGTH; i++) {
          byte red = 255 >> i;
          values[i] = pixel_color(red, 0, 0);
        }
        break;
      }
      case 2: {
        for (int i = 0; i < SNAKE2_LENGTH; i++) {
          byte green = 255 >> i;
          values[i] = pixel_color(0, green, 0);
        }
        break;
      }
      case 3: {
        for (int i = 0; i < SNAKE2_LENGTH; i++) {
          byte blue = 255 >> i;
          values[i] = pixel_color(0, 0, blue);
        }
        break;
      }
      case 4: {
        for (int i = 0; i < SNAKE2_LENGTH; i++) {
          byte color = 255 >> i;
          values[i] = pixel_color(color, color, color);
        }
        break;
      }
    }

    /* Clear the tail */
    byte tri, vert;
    byte nextIndex;
    boolean found = false;

    /* Choose the next location */
    for (byte i = 0; i < SNAKE2_LENGTH; i++) {
      byte activeIndex = (currentIndex + SNAKE2_LENGTH - i) % SNAKE2_LENGTH;
      if (snakeTriangles[activeIndex] ==  Triangle::NO_ID) continue;
      Triangle *current = &triangles[snakeTriangles[activeIndex]];
      byte currentVertex = snakeVertices[activeIndex];
      if (currentVertex == Triangle::NO_VERTEX) continue;
      tri = Triangle::NO_ID;

      byte startDirection = random(0, 4);
      for (byte direction = 0; direction < 4; direction++) {
        switch ((startDirection + direction) % 4) {
          case 0:
          default: {
            // Triangle to the left
            Triangle *t = current->leftOfVertex(currentVertex);
            if (t == NULL) continue;
            tri = t->id;
            vert = triangles[tri].matchVertexRight(current, 
                                                   currentVertex);	
            break;
          }
          case 1: {
            // Triangle to the right
            Triangle *t = current->rightOfVertex(currentVertex);
            if (t == NULL) continue;
            tri = t->id;
            vert = triangles[tri].matchVertexLeft(current, 
                                                  currentVertex);
            break;
          }
          case 2: {
            // Same triangle, vertex to the left
            vert = (currentVertex + Triangle::NUM_EDGES - 1) % Triangle::NUM_EDGES;
            tri = snakeTriangles[activeIndex];
            break;
          }
          case 3: {
            // Same triangle, vertex to the right
            vert = (currentVertex + 1) % Triangle::NUM_EDGES;
            tri = snakeTriangles[activeIndex];
            break;
          }
        }

        if ((triangles[tri].hasLeds()) &&
            (triangles[tri].leds[vert].color() == config->bgColor)) {
          // Verify that the choosen vertex is dark
          found = true;
          goto FOUND;
        }
      }
    }

  FOUND:

    if (!found) {
      currentIndex = (byte)-1;
      DEBUG4_PRINTLN("End of snake");
      return;
    }

    if (currentIndex == 0) {
      nextIndex = SNAKE2_LENGTH - 1;
    } else {
      nextIndex = currentIndex - 1;
    }
    if ((snakeTriangles[nextIndex] < size) && 
        (snakeVertices[nextIndex] < Triangle::NUM_VERTICES)) 
      triangles[snakeTriangles[nextIndex]].setColor(snakeVertices[nextIndex], 
                                                    config->bgColor);

    /* Move the array index*/
    currentIndex = nextIndex;
    snakeTriangles[currentIndex] = tri;
    snakeVertices[currentIndex] = vert;

    /* Set the led values */
    for (byte i = 0; i < SNAKE2_LENGTH; i++) {
      byte valueIndex = (i + SNAKE2_LENGTH - currentIndex) % SNAKE2_LENGTH;
      if (snakeTriangles[i] != (byte)-1) {
	
        triangles[snakeTriangles[i]].setColor(snakeVertices[i],
                                              values[valueIndex]);
      }
    }

    DEBUG5_VALUE(" i:", currentIndex);
    DEBUG5_VALUE(" tri:", tri);
    DEBUG5_VALUELN(" vert:", vert);
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
    DEBUG4_HEXVAL("color=", arg->fgColor);
    DEBUG4_HEXVAL(" getColor=", triangles[0].getColor());
    DEBUG4_HEXVAL(" getRed=", triangles[0].getRed());
    DEBUG4_HEXVAL(" getGreen=", triangles[0].getGreen());
    DEBUG4_HEXVALLN(" getBlue=", triangles[0].getBlue());
  }
}

/* Shift the color of a vertex randomly around */
void trianglesVertexShift(Triangle *triangles, int size, int periodms,
                          boolean init, pattern_args_t *arg) {
  static byte mode = 0;
  byte num_modes = 4;

  if (init) {
    next_time = millis() + periodms;
    randomTriangles(triangles, size);
    //    triangles[0].setColor(0, 255, 0, 0);
    mode = 0;
  }

  if (millis() > next_time) {
    next_time += periodms;

    mode = random(0, num_modes);

    for (int tri = 0; tri < size; tri++) {
      for (byte vertex = 0; vertex < Triangle::NUM_VERTICES; vertex++) {
        Triangle *sourceT;
        byte sourceV;
        uint32_t color;

        for (int i = 0; i < num_modes; i++ ) {
          switch ((i + mode) % num_modes) {
            case 0: {
              // Set the color of the vertex to that of the vertex to its right
              movementCornerCCW(&triangles[tri], vertex, &sourceT, &sourceV);
              break;
            }
            case 1: {
              sourceT = &triangles[tri];
              sourceV = VERTEX_CW(vertex);
              break;
            }
            case 2: {
              // Set the color of the vertex to that of the vertex to its right
              movementCornerCCW(&triangles[tri], vertex, &sourceT, &sourceV);
              break;
            }
            case 3: {
              sourceT = &triangles[tri];
              sourceV = VERTEX_CW(vertex);
              break;
            }
          }

          if (sourceT) color = sourceT->getColor(sourceV);
          else color = 0;
          if (color != 0) break;
        }

       	triangles[tri].setColor(vertex, color);
      }
    }
    mode++;
  }
}
    
void trianglesVertexMerge(Triangle *triangles, int size, int periodms,
                          boolean init, pattern_args_t *arg) {
  static unsigned long next_mutation;
  static byte mutation = 0;

  if (init) {
    next_time = millis() + periodms;
    next_mutation = next_time + periodms * 20;
    //    wheelTriangles(triangles, size);
    clearTriangles(triangles, size);
  }
  
  if (millis() > next_time) {
    next_time += periodms;

    mergeAdjacent(triangles, size);
  }

  if (millis() > next_mutation) {
    next_mutation += periodms * 40;
    //mutation++; triangles[random(0, size)].setColor(pixel_wheel(mutation));
    mutation += 11; triangles[random(0, size)].setColor(pixel_primary(mutation));
  }
}

void trianglesVertexMergeFade(Triangle *triangles, int size, int periodms,
			      boolean init, pattern_args_t *arg) {
  static unsigned long next_mutation;
  static unsigned long next_fade;
  static byte mutation = 0;

  if (init) {
    next_time = millis() + periodms;
    next_mutation = next_time + periodms * 20;
    next_fade = next_time + periodms * 20;

    //    wheelTriangles(triangles, size);
    clearTriangles(triangles, size);
  }
  
  if (millis() > next_time) {
    next_time += periodms;

    mergeAdjacent(triangles, size);
  }

  if (millis() > next_fade) {
    next_fade += periodms * 8;
    incrementAll(triangles, size, -1, -1, -1);
  }

  if (millis() > next_mutation) {
    next_mutation += periodms * 40;
    //mutation++; triangles[random(0, size)].setColor(pixel_wheel(mutation));
    mutation += 11; triangles[random(0, size)].setColor(pixel_primary(mutation));
    //triangles[random(0, size)].setColor(255, 0, 0);
  }
}

