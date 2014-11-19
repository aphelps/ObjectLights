#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "PixelUtil.h"
#include "SquareStructure.h"

// XXX: Relying on an array allocated by the main sketch feels icky
extern Square *squares;


Square::Square(unsigned int _id) {
  updated = false;
  mark = 0;
  id = _id;

  for (int e = 0; e < NUM_EDGES; e++) {
    edges[e] = NO_ID;
  }

  for (int v = 0; v < NUM_VERTICES; v++) {
    for (int o = 0; o < VERTEX_ORDER; o++) {
      vertices[v][o] = NULL;
    }
  }

  DEBUG_VALUELN(DEBUG_MID, "Created Square ", id);
}


Square *Square::getEdge(byte edge) {
  return &squares[edges[edge]];
}

void Square::setEdge(byte edge, Square *square) {
  edges[edge] = square->id;
}

Square *Square::getVertex(byte vertex, byte index) {
  return &squares[vertices[vertex][index]];
}

void Square::setVertex(byte vertex, byte index, Square *square) {
  vertices[vertex][index] = square->id;
}

void Square::setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2,
			  uint16_t p3, uint16_t p4, uint16_t p5,
			  uint16_t p6, uint16_t p7, uint16_t p8) {
  leds[0].pixel = p0;
  leds[1].pixel = p1;
  leds[2].pixel = p2;
  leds[3].pixel = p3;
  leds[4].pixel = p4;
  leds[5].pixel = p5;
  leds[6].pixel = p6;
  leds[7].pixel = p7;
  leds[8].pixel = p8;
}

void Square::setLedPixel(byte led, uint16_t pixel) {
  leds[led].pixel = pixel;
}

void Square::setColor(byte r, byte g, byte b) {
  if (hasLeds()) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setColor(r, g, b);
    }
    updated = true;
  }
}

void Square::setColor(byte led, byte r, byte g, byte b) {
  if (hasLeds()) {
    leds[led].setColor(r, g, b);
    updated = true;
  }
}

void Square::setColor(uint32_t c) {
  if (hasLeds()) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setColor(c);
    }
    updated = true;
  }
}

void Square::setColor(byte led, uint32_t c) {
  if (hasLeds()) {
    leds[led].setColor(c);
    updated = true;
  }
}

/* Set the color of an led based on row and column */
void Square::setColorRC(byte col, byte row, uint32_t c) {
  setColor(row * 3 + col, c);
}

/* Set the color of an entire column */
void Square::setColorColumn(byte col, uint32_t c) {
  for (byte row = 0; row < SQUARE_LED_ROWS; row++) {
    setColorRC(col, row, c);
  }
}

/* Set the bottom of the column's color and shift other rows up */
void Square::shiftColumnUp(byte col, uint32_t c) {
  for (byte row = 0; row < SQUARE_LED_ROWS - 1; row++) {
    uint32_t current = getColor((row+1) * 3 + col);
    setColorRC(col, row, current);
  }
  setColorRC(col, SQUARE_LED_ROWS - 1, c);
}

/* Set the top of the column's color and shift other rows down */
void Square::shiftColumnDown(byte col, uint32_t c) {
  for (byte row = 1; row < SQUARE_LED_ROWS; row++) {
    uint32_t current = getColor((row-1) * 3 + col);
    setColorRC(col, row, current);
  }
  setColorRC(col, 0, c);
}


/* Set the color of an entire row */
void Square::setColorRow(byte row, uint32_t c) {
  for (byte col = 0; col < SQUARE_LED_COLS; col++) {
    setColorRC(col, row, c);
  }
}

/* Set the color of the indicated edge */
void Square::setColorEdge(byte edge, uint32_t c) {
  switch (edge) {
  case TOP: setColorRow(0, c); break;
  case RIGHT: setColorColumn(2, c); break;
  case BOTTOM: setColorRow(2, c); break;
  case LEFT: setColorColumn(0, c); break;
  }
}

extern PixelUtil pixels;

uint32_t Square::getColor() {
  if (hasLeds()) {
    return pixels.getColor(leds[0].pixel);
  } else {
    return 0;
  }
}

uint32_t Square::getColor(byte led) {
  if (hasLeds()) {
    return pixels.getColor(leds[led].pixel);
  } else {
    return 0;
  }
}

byte Square::getRed() {
  if (hasLeds()) {
    return pixel_red(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Square::getRed(byte vertex) {
  if (hasLeds()) {
    return pixel_red(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}


byte Square::getGreen() {
  if (hasLeds()) {
    return pixel_green(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Square::getGreen(byte vertex) {
  if (hasLeds()) {
    return pixel_green(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}


byte Square::getBlue() {
  if (hasLeds()) {
    return pixel_blue(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Square::getBlue(byte vertex) {
  if (hasLeds()) {
    return pixel_blue(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}

/******************************************************************************
 * Geometry functions
 */

/* Returns the edge of this square adjacent to the target square */
byte Square::matchEdge(Square *square) {
  for (byte edge = 0; edge < NUM_EDGES; edge++) {
    if (getEdge(edge) == square) return edge;
  }
  return NO_EDGE;
}

/*
 *  Return the index of an LED within a given edge.  The LEDs are indexed in
 *  a clockwise ordering:
 *
 *   0  1  2
 *  +-------+
 * 2|       |0
 * 1|       |1
 * 0|       |2
 *  +-------+
 *   2  1  0
 */
byte Square::getEdgeIndex(byte edge, byte led) {
  switch (led) {
  case 0: {
    switch (edge) {
    case TOP: return 0; break;
    case LEFT: return 2; break;
    }
    break;
  }
  case 1: {
    switch (edge) {
    case TOP: return 1; break;
    }
    break;
  }
  case 2: {
    switch (edge) {
    case TOP: return 2; break;
    case RIGHT: return 0; break;
    }
    break;
  }
  case 3: {
    switch (edge) {
    case LEFT: return 1; break;
    }
    break;
  }
  case 5: {
    switch (edge) {
    case RIGHT: return 1; break;
    }
    break;
  }
  case 6: {
    switch (edge) {
    case LEFT: return 0; break;
    case BOTTOM: return 2; break;
    }
    break;
  }
  case 7: {
    switch (edge) {
    case BOTTOM: return 1; break;
    }
    break;
  }
  case 8: {
    switch (edge) {
    case BOTTOM: return 0; break;
    case RIGHT: return 2; break;
    }
    break;
  }
  }
  return NO_INDEX;
}

/* Returns the led at the indicated index in an edge */
byte Square::ledInEdge(byte edge, byte index) {
  // index should be 0-2
  switch (edge) {
  case TOP: {
    return index; //  0 1 2
  }
  case RIGHT: {
    return 2 + 3*index; // 2 5 8
  }
  case BOTTOM: {
    return 8 - index; // 8 7 6
  }
  case LEFT: {
    return 6 - 3*index; // 6 3 0
  }
  }
  return NO_LED;
}

/*
 *  Returns the led in this square adjacent to an led in neighboring square 
 *
 *  +-------+-------+
 *  |       |       |
 *  |       |       |
 *  |      X|?      |
 *  +-------+-------+
 */

byte Square::matchLED(Square *square, byte led) {
  // Determine which edge matches
  byte my_edge = matchEdge(square);

  // Determine the edge on the adjacent square and the index of the led in
  // that edge.
  byte other_edge = square->matchEdge(this);
  byte other_index = square->getEdgeIndex(other_edge, led);

  byte my_index = NO_LED;
  switch (other_index) {
  case 0: my_index = 2; break;
  case 1: my_index = 1; break;
  case 2: my_index = 0; break;
  default: return NO_LED; break;
  }

  byte match = ledInEdge(my_edge, my_index);

  return match;
}

/*
 * Return the led in this square in the given direction
 */
byte Square::ledInDirection(byte led, byte direction) {
 switch (direction) {
 case TOP: {
   if (led >= 3) return (led - 3);
   break;
 }

 case RIGHT: {
   if ((led % 3) != 2) return (led + 1);
   break;
 }

 case BOTTOM: {
   if (led <= 5) return (led + 3);
   break;
 }

 case LEFT: {
   if ((led % 3) != 0) return (led - 1);
   break;
 }
 }

 return NO_LED;
}

/*
 * Returns the square and led in a given direction from a led in this square
 */
uint16_t Square::ledTowards(byte led, byte direction) {
  /*
   * Get LED in the appropriate direction, which will also detect if the led
   * is the last in that direction.
   */
  byte next_led = ledInDirection(led, direction);
  if (next_led != NO_LED) return FACE_AND_LED(id, next_led);

  /*
   * Next LED is on the next face
   */
  Square *next_square = getEdge(direction);
  next_led = next_square->matchLED(this, led);
  return FACE_AND_LED(next_square->id, next_led);
}

/*
 * Returns the square and led in away from a neighboring square from a led in
 * this square.
 */
uint16_t Square::ledAwayFrom(Square *square, byte led) {
  /*
   * Get LED in the appropriate direction, which will also detect if the led
   * is the last in that direction.
   */
  byte direction = matchEdge(square);
  if (direction == NO_DIRECTION) {
    DEBUG_PRINTLN(DEBUG_HIGH, "ledAwayFrom: invalid");
    return (uint16_t)-1;
  }
  direction = (direction + 2) % Square::NUM_EDGES;
  return ledTowards(led, direction);
}



/******************************************************************************
 * Serialization functions
 */
int Square::toBytes(byte *bytes, int size) {
  int i = 0;

  // Write out the ID
  bytes[i++] = id;

  // Write out the IDs of the adjacent edges
  for (int face = 0; face < NUM_EDGES; face++) {
    bytes[i++] = getEdge(face)->id;
  }

  // Write out the pixel values
  for (int led = 0; led < NUM_LEDS; led++ ) {
    bytes[i++] = leds[led].pixel;
  }

  return i;
}

void Square::fromBytes(byte *bytes, int size, Geometry *squares, int numSquares) {
  int i = 0;

  // Read the ID
  id = bytes[i++];

  // Read the IDs of the adjacent edges
  for (int face = 0; face < NUM_EDGES; face++) {
    int id = bytes[i++];
    edges[face] = id;
  }

  // Read the pixel values
  for (int led = 0; led < NUM_LEDS; led++ ) {
    leds[led].pixel = bytes[i++];
  }
}

#ifdef DEBUG_LEVEL
void Square::print(byte level) {

}
#endif

/* Send updated values to a Pixel chain */
void updateSquarePixels(Square *squares, int numSquares,
			  PixelUtil *pixels) {
  boolean update = false;
  int updated = 0;
  for (int tri = 0; tri < numSquares; tri++) {
    if (squares[tri].updated) {
      update = true;
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	pixels->setPixelRGB(&(squares[tri].leds[led]));
      }
      squares[tri].updated = false;
      updated++;
    }
  }

  if (update) {
    pixels->update();
    DEBUG_VALUELN(DEBUG_TRACE, "Updated squares:", updated);
  }
}



/******************************************************************************
 * Topology construction helper functions
 */

void makeEdge(Square *squares, int tri, int edge, int neighbor) {
  squares[tri].setEdge(edge, &squares[neighbor]);
}

/******************************************************************************
 * Construct a cube
 *
 * Flattened Layout:
 * +---+
 * | 4 |
 * +---+---+---+---+
 * | 0 | 1 | 2 | 3 |
 * +---+---+---+---+
 * | 5 |
 * +---+
 *
 */


Square squareArray[6];
Square* buildCube(int *numSquares, int numLeds, int firstLed) {
  int squareCount = 6;

  Square *squares = &(squareArray[0]);

  int led = firstLed;

  // XXX: Build topology
  *numSquares = 0;
  makeEdge(squares, 0, Square::TOP,    4);
  makeEdge(squares, 0, Square::RIGHT,  1);
  makeEdge(squares, 0, Square::BOTTOM, 5);
  makeEdge(squares, 0, Square::LEFT,   3);
  squares[*numSquares].setLedPixels(led + 2, led + 1, led + 0,
				    led + 3, led + 4, led + 5,
				    led + 8, led + 7, led + 6);
  squares[*numSquares].id = *numSquares;
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 1, Square::TOP,    4);
  makeEdge(squares, 1, Square::RIGHT,  2);
  makeEdge(squares, 1, Square::BOTTOM, 5);
  makeEdge(squares, 1, Square::LEFT,   0);
  squares[*numSquares].setLedPixels(led + 17, led + 16, led + 15,
				    led + 12, led + 13, led + 14,
				    led + 11, led + 10, led + 9);
  squares[*numSquares].id = *numSquares;
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 2, Square::TOP,    4);
  makeEdge(squares, 2, Square::RIGHT,  3);
  makeEdge(squares, 2, Square::BOTTOM, 5);
  makeEdge(squares, 2, Square::LEFT,   1);
  squares[*numSquares].setLedPixels(led + 20, led + 19, led + 18,
				    led + 21, led + 22, led + 23,
				    led + 26, led + 25, led + 24);
  squares[*numSquares].id = *numSquares;
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 3, Square::TOP,    4);
  makeEdge(squares, 3, Square::RIGHT,  0);
  makeEdge(squares, 3, Square::BOTTOM, 5);
  makeEdge(squares, 3, Square::LEFT,   2);
  squares[*numSquares].setLedPixels(led + 35, led + 34, led + 33,
				    led + 30, led + 31, led + 32,
				    led + 29, led + 28, led + 27);
  squares[*numSquares].id = *numSquares;
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 4, Square::TOP,    2);
  makeEdge(squares, 4, Square::RIGHT,  1);
  makeEdge(squares, 4, Square::BOTTOM, 0);
  makeEdge(squares, 4, Square::LEFT,   3);
  squares[*numSquares].setLedPixels(led + 44, led + 39, led + 38,
				    led + 43, led + 40, led + 37,
				    led + 42, led + 41, led + 36);
  squares[*numSquares].id = *numSquares;
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 5, Square::TOP,    0);
  makeEdge(squares, 5, Square::RIGHT,  1);
  makeEdge(squares, 5, Square::BOTTOM, 2);
  makeEdge(squares, 5, Square::LEFT,   3);
  squares[*numSquares].id = *numSquares;
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

 CUBE_DONE:
  return squares;
}
