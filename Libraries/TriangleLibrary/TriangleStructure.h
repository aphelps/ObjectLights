/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Definition of an ObjectLights triangle
 *
 * -----------------------------------------------------------------------------
 *
 * A triangle is defined such that it has sides and vertices numbered 0-2,
 * starting at its "top" and going clockwise from there.
 *
 *          0
 *          ^
 *         / \
 *        / 0 \
 *     2 /     \ 0
 *      / 2   1 \
 *     /_________\
 *    2     1     1
 *
 * A triangle may have a single neighbor along each edge, but multiple
 * neighbors at each vertex.  If another triangle is a neighbor on an edge
 * then it cannot also neighbor at a vertex.  Vertex neighbors are also indexed
 * clockwise (or left to right if triangle is positioned with vertex on top).
 */
#ifndef TRIANGLESTRUCTURE
#define TRIANGLESTRUCTURE

#include "PixelUtil.h"

#include "Geometry.h"

class Triangle : public Geometry {
 public:
  /* Geometry values */
  static const byte NUM_EDGES = 3;
  static const byte NUM_VERTICES = 3;
  static const byte VERTEX_ORDER = 2;

  Triangle() {};
  Triangle(unsigned int id);

  Triangle *getEdge(byte edge);
  void setEdge(byte edge, Triangle *tri);

  Triangle *getVertex(byte vertex, byte index);
  void setVertex(byte vertex, byte index, Triangle *tri);

  RGB* getLED(byte vertex);
  void setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2);

  void setColor(byte r, byte g, byte b);
  void setColor(uint32_t c);
  void setColor(byte led, byte r, byte g, byte b);
  void setColor(byte led, uint32_t c);

  uint32_t getColor();
  uint32_t getColor(byte led);
  byte getRed();
  byte getRed(byte vertex);
  byte getGreen();
  byte getGreen(byte vertex);
  byte getBlue();
  byte getBlue(byte vertex);

  byte matchVertex(Triangle *neighbor);
  Triangle *leftOfVertex(byte vertex);
  Triangle *rightOfVertex(byte vertex);
  byte matchVertexLeft(Triangle *neighbor, byte vertex);
  byte matchVertexRight(Triangle *neighbor, byte vertex);

  void print(byte level);

  boolean hasLeds() { return (leds[0].pixel != NO_LED); }

  // Variables - be careful of object size
  boolean updated; // XXX - Can this be determined as well?
  byte id;
  RGB leds[3]; // XXX - Change pixel addresses to bytes
  byte mark;

 private:
  byte edges[NUM_EDGES];
  byte vertices[NUM_VERTICES][VERTEX_ORDER];
};

/* Send updated values to a Pixel chain */
void updateTrianglePixels(Triangle *triangles, int numTriangles,
			  PixelUtil *pixels);

/* Allocate and return a fully connected icosohedron */
Triangle* buildIcosohedron(int *numTriangles, int numLeds);

/* Allocate and return a fully connected cylinder */
Triangle* buildCylinder(int *numTriangles, int numLeds);

/* Macros for rotating around the vertices of a triangle */
#define VERTEX_CW(v) ((v + 1) % Triangle::NUM_VERTICES)
#define VERTEX_CCW(v) ((v + Triangle::NUM_VERTICES - 1) % Triangle::NUM_VERTICES)

#endif
