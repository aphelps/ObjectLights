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
  static const byte NUM_LEDS = 3;

  Triangle() {};
  Triangle(geo_id_t id);

  Triangle *getEdge(byte edge);
  void setEdge(byte edge, Triangle *tri);
  void setEdge(byte edge, byte neighbor);

  Triangle *getVertex(byte vertex, byte index);
  geo_id_t getVertexID(byte vertex, byte index);

  PRGB* getLED(byte vertex);
  void setLedPixel(uint8_t led, uint16_t pixel);
  void setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2);

  void setColor(byte r, byte g, byte b);
  void setColor(uint32_t c);
  void setColor(byte led, byte r, byte g, byte b);
  void setColor(byte led, uint32_t c);

  uint32_t getColor();
  uint32_t getColor(byte vertex);
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

  void print();

  /* Serialization functions */
  int toBytes(byte *bytes, int size);
  void fromBytes(byte *bytes, int size, Geometry *triangles, 
                 geo_id_t numTriangles);

  /* Functions for returning constants for superclass */
  byte numEdges() { return NUM_EDGES; }
  byte numVertices() { return NUM_VERTICES; }
  byte numLeds() { return NUM_LEDS; }

  static boolean verifyTriangleStructure(Triangle *triangles,
                                         int numTriangles, 
                                         geo_led_t numLeds);

  /*
   * Variables - be careful of object size
   */
  PRGB leds[NUM_LEDS];

 protected:
  geo_id_t edges[NUM_EDGES];
};

/* Send updated values to a Pixel chain */
boolean updateTrianglePixels(Triangle *triangles, int numTriangles,
			  PixelUtil *pixels);


// XXX: This should not be hard coded
#ifndef TRI_ARRAY_SIZE
  #define TRI_ARRAY_SIZE 20
  #warning Using default triangle array size of 20
#endif
Triangle* initTriangles(int triangleCount);

/* Allocate and return a fully connected icosohedron */
Triangle* buildIcosohedron(int *numTriangles, int numLeds);

/* Allocate and return a fully connected cylinder */
Triangle* buildCylinder(int *numTriangles, int numLeds);

/* Macros for rotating around the vertices of a triangle */
#define VERTEX_CW(v) ((v + 1) % Triangle::NUM_VERTICES)
#define VERTEX_CCW(v) ((v + Triangle::NUM_VERTICES - 1) % Triangle::NUM_VERTICES)

/* Serialization structure */
typedef struct {
  byte id;
  byte edges[Triangle::NUM_EDGES];
  byte leds[Triangle::NUM_LEDS];
} triangle_config_t;

/* Read or write out an entire structure */
int readTriangleStructure(int offset, Triangle **triangles_ptr,
                         int *numTriangles);
int writeTriangleStructure(Triangle *triangles, int numTriangles, 
                           int offset);

#endif
