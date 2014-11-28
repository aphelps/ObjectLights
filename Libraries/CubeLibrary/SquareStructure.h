/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Definition of an ObjectLights square
 *
 * -----------------------------------------------------------------------------
 *
 *   3      0       0
 *    +------------+
 *    | 0    1    2|
 *    |            |
 *   3| 3    4    5|1
 *    |            |
 *    | 6    7    8|
 *    +------------+
 *   2      2       1
 */
#ifndef SQUARESTRUCTURE
#define SQUARESTRUCTURE

#include "PixelUtil.h"
#include "Geometry.h"

class Square : public Geometry {
 public:
  /* Geometry values */
  static const byte NUM_EDGES    = 4;
  static const byte NUM_VERTICES = 4;
  static const byte VERTEX_ORDER = 1;

  static const byte SQUARE_LED_ROWS = 3;
  static const byte SQUARE_LED_COLS = 3;
  static const byte NUM_LEDS = SQUARE_LED_ROWS * SQUARE_LED_COLS;

  /* Edge values */
  static const byte TOP = 0;
  static const byte RIGHT = 1;
  static const byte BOTTOM = 2;
  static const byte LEFT = 3;

  /* LED values */
  static const byte CENTER = 4;

  Square() {};
  Square(unsigned int id);

  Square *getEdge(byte edge);
  void setEdge(byte edge, Square *square);

  Square *getVertex(byte vertex, byte index);
  void setVertex(byte vertex, byte index, Square *square);

  void setLedPixel(byte led, uint16_t pixel);
  void setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2,
		    uint16_t p3, uint16_t p4, uint16_t p5,
		    uint16_t p6, uint16_t p7, uint16_t p8);

  void setColor(byte r, byte g, byte b); // Set color of entire square
  void setColor(uint32_t c);             // Set color of entire square
  void setColor(byte led, byte r, byte g, byte b);
  void setColor(byte led, uint32_t c);

  void setColorRC(byte col, byte row, uint32_t c);
  void setColorColumn(byte col, uint32_t c);
  void setColorRow(byte row, uint32_t c);
  void setColorEdge(byte edge, uint32_t c);
  void shiftColumnUp(byte col, uint32_t c);
  void shiftColumnDown(byte col, uint32_t c);

  uint32_t getColor();
  uint32_t getColor(byte led);
  byte getRed();
  byte getRed(byte vertex);
  byte getGreen();
  byte getGreen(byte vertex);
  byte getBlue();
  byte getBlue(byte vertex);

  /* Geometry functions */
  byte matchEdge(Square *square);
  byte matchLED(Square *square, byte led);
  byte getEdgeIndex(byte edge, byte led);
  byte ledInEdge(byte edge, byte index);
  byte ledInDirection(byte led, byte direction);

  uint16_t ledAwayFrom(Square *square, byte led);
  uint16_t ledTowards(byte led, byte direction);

  void print(byte level);

  /* Serialization functions */
  int toBytes(byte *bytes, int size);
  void fromBytes(byte *bytes, int size, Geometry *squares, int numSquares);

  /* Functions for returning constants for superclass */
  byte numEdges() { return NUM_EDGES; }
  byte numVertices() { return NUM_VERTICES; }
  byte numLeds() { return NUM_LEDS; }

  // Variables - be careful of object size
  RGB leds[NUM_LEDS];

 protected:
  byte edges[NUM_EDGES];
  byte vertices[NUM_VERTICES][VERTEX_ORDER];
};

/* Send updated values to a Pixel chain */
void updateSquarePixels(Square *squares, int numSquares,
			  PixelUtil *pixels);

/* Allocate and return a fully connected cube */
#define CUBE_FRONT  0
#define CUBE_RIGHT  1
#define CUBE_BACK   2
#define CUBE_LEFT   3
#define CUBE_TOP    4
#define CUBE_BOTTOM 5
Square* buildCube(int *numSquares, int numLeds, int firstLed);

// Store both face and led in a uint16_t
#define FACE_AND_LED(face, led) (face << 8 | led)
#define LED_FROM_COMBO(faceled) (faceled & 0xF)
#define FACE_FROM_COMBO(faceled) ((faceled >> 8) & 0xF)

// Store mask of all faces and leds in a uint16_t
#define FACE_LED_MASK(faces, leds) ((faces << Square::NUM_LEDS) | leds)
#define LEDS_FROM_MASK(mask) (mask ^ (0xFFFFFFFF << Square::NUM_LEDS))
#define FACES_FROM_MASK(mask) (mask >> Square::NUM_LEDS)

#define REV_DIRECTION(dir) ((dir + 2) % Square::NUM_EDGES)


#endif
