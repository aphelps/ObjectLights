#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "PixelUtil.h"
#include "SquareStructure.h"

Square::Square(unsigned int _id) {
  hasLeds = false;
  updated = false;
  mark = 0;
  id = _id;

  for (int e = 0; e < NUM_EDGES; e++) {
    edges[e] = NULL;
  }

  for (int v = 0; v < NUM_VERTICES; v++) {
    for (int o = 0; o < VERTEX_ORDER; o++) {
      vertices[v][o] = NULL;
    }
  }

  DEBUG_VALUELN(DEBUG_MID, "Created Square ", id);
}


Square *Square::getEdge(byte edge) {
  return edges[edge];
}

void Square::setEdge(byte edge, Square *square) {
  edges[edge] = square;
}

Square *Square::getVertex(byte vertex, byte index) {
  return vertices[vertex][index];
}

void Square::setVertex(byte vertex, byte index, Square *square) {
  vertices[vertex][index] = square;
}

void Square::setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2,
			  uint16_t p3, uint16_t p4, uint16_t p5,
			  uint16_t p6, uint16_t p7, uint16_t p8) {
  hasLeds = true;
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
  hasLeds = true;
  leds[led].pixel = pixel;
}

void Square::setColor(byte r, byte g, byte b) {
  if (hasLeds) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setColor(r, g, b);
    }
    updated = true;
  }
}

void Square::setColor(byte led, byte r, byte g, byte b) {
  if (hasLeds) {
    leds[led].setColor(r, g, b);
    updated = true;
  }
}

void Square::setColor(uint32_t c) {
  if (hasLeds) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setColor(c);
    }
    updated = true;
  }
}

void Square::setColor(byte led, uint32_t c) {
  if (hasLeds) {
    leds[led].setColor(c);
    updated = true;
  }
}

extern PixelUtil pixels;

uint32_t Square::getColor() {
  if (hasLeds) {
    return pixels.getColor(leds[0].pixel);
  } else {
    return 0;
  }
}

uint32_t Square::getColor(byte led) {
  if (hasLeds) {
    return pixels.getColor(leds[led].pixel);
  } else {
    return 0;
  }
}

byte Square::getRed() {
  if (hasLeds) {
    return pixel_red(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Square::getRed(byte vertex) {
  if (hasLeds) {
    return pixel_red(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}


byte Square::getGreen() {
  if (hasLeds) {
    return pixel_green(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Square::getGreen(byte vertex) {
  if (hasLeds) {
    return pixel_green(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}


byte Square::getBlue() {
  if (hasLeds) {
    return pixel_blue(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Square::getBlue(byte vertex) {
  if (hasLeds) {
    return pixel_blue(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}

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
 *     +---+
 *     | 4 |
 * +---+---+---+---+
 * | 0 | 1 | 2 | 3 |
 * +---+---+---+---+
 *     | 5 |
 *     +---+
 *
 */


Square squareArray[6];
Square* buildCube(int *numSquares, int numLeds, int firstLed) {
  int squareCount = 6;

  Square *squares = &(squareArray[0]);

  int led = firstLed;

  // XXX: Build topology
  *numSquares = 0;
  makeEdge(squares, 0, 0, 4);
  makeEdge(squares, 0, 1, 3);
  makeEdge(squares, 0, 2, 5);
  makeEdge(squares, 0, 3, 1);
  squares[*numSquares].setLedPixels(led + 2, led + 1, led + 0,
				    led + 3, led + 4, led + 5,
				    led + 8, led + 7, led + 6);
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 1, 0, 4);
  makeEdge(squares, 1, 1, 0);
  makeEdge(squares, 1, 2, 5);
  makeEdge(squares, 1, 3, 2);
  squares[*numSquares].setLedPixels(led + 17, led + 16, led + 15,
				    led + 12, led + 13, led + 14,
				    led + 11, led + 10, led + 9);
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 2, 0, 4);
  makeEdge(squares, 2, 1, 1);
  makeEdge(squares, 2, 2, 5);
  makeEdge(squares, 2, 3, 3);
  squares[*numSquares].setLedPixels(led + 20, led + 19, led + 18,
				    led + 21, led + 22, led + 23,
				    led + 26, led + 25, led + 24);
 (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 3, 0, 4);
  makeEdge(squares, 3, 1, 2);
  makeEdge(squares, 3, 2, 5);
  makeEdge(squares, 3, 3, 0);
  squares[*numSquares].setLedPixels(led + 35, led + 34, led + 33,
				    led + 30, led + 31, led + 32,
				    led + 29, led + 28, led + 27);
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 4, 0, 0);
  makeEdge(squares, 4, 1, 1);
  makeEdge(squares, 4, 2, 2);
  makeEdge(squares, 4, 3, 3);
  squares[*numSquares].setLedPixels(led + 44, led + 39, led + 38,
				    led + 43, led + 40, led + 37,
				    led + 42, led + 41, led + 36);
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

  makeEdge(squares, 5, 0, 2);
  makeEdge(squares, 5, 1, 1);
  makeEdge(squares, 5, 2, 0);
  makeEdge(squares, 5, 3, 3);
  (*numSquares)++;
  if (*numSquares == squareCount) goto CUBE_DONE;

 CUBE_DONE:
  return squares;
}
