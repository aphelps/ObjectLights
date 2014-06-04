#ifndef TRIANGLESTRUCTURE
#define TRIANGLESTRUCTURE

#include "PixelUtil.h"

/*
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

#define TRIANGLE_NUM_EDGES 3
#define TRIANGLE_NUM_VERTICES 3
#define TRIANGLE_VERTEX_ORDER 2
class Triangle {
 public:

  static const byte NO_LED = (byte)-1;
  static const byte NO_VERTEX = (byte)-1;
  static const byte NO_ID = (byte)-1;

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
  byte edges[TRIANGLE_NUM_EDGES];
  byte vertices[TRIANGLE_NUM_VERTICES][TRIANGLE_VERTEX_ORDER];
};

/* Send updated values to a Pixel chain */
void updateTrianglePixels(Triangle *triangles, int numTriangles,
			  PixelUtil *pixels);

/* Allocate and return a fully connected icosohedron */
Triangle* buildIcosohedron(int *numTriangles, int numLeds);

/* Allocate and return a fully connected cylinder */
Triangle* buildCylinder(int *numTriangles, int numLeds);

/* Macros for rotating around the vertices of a triangle */
#define VERTEX_CW(v) ((v + 1) % TRIANGLE_NUM_VERTICES)
#define VERTEX_CCW(v) ((v + TRIANGLE_NUM_VERTICES - 1) % TRIANGLE_NUM_VERTICES)

#endif
