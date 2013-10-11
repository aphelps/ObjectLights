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
 *        / * \
 *     2 /     \ 0
 *      / *   * \
 *     /_________\
 *    2     1     1
 *
 * A triangle may have a single neighbor along each edge, but multiple
 * neighbors at each vertex.  If another triangle is a neighbor on an edge
 * then it cannot also neighbor at a vertex.  Vertex neighbors are also indexed
 * clockwise (or left to right if triangle is positioned with vertex on top).
 */

class Triangle {
 public:

  Triangle(unsigned int id);

  Triangle *getEdge(byte edge);
  void setEdge(byte edge, Triangle *tri);

  Triangle *getVertex(byte vertex, byte index);
  void setVertex(byte vertex, byte index, Triangle *tri);

  RGB* getLED(byte vertex);
  void setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2);

  void setColor(byte r, byte g, byte b);

  boolean hasLeds;

 private:

  unsigned int id;

  RGB leds[3];
  Triangle *edges[3];
  Triangle *vertices[3][2];
};

/* Allocate and return a fully connected icosohedron */
Triangle** buildIcosohedron();

#endif
