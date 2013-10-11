#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "PixelUtil.h"
#include "TriangleStructure.h"

Triangle::Triangle(unsigned int _id) {
  hasLeds = false;
  id = _id;
}

Triangle *Triangle::getEdge(byte edge) {
  return edges[edge];
}

void Triangle::setEdge(byte edge, Triangle *tri) {
  edges[edge] = tri;
}

Triangle *Triangle::getVertex(byte vertex, byte index) {
  return vertices[vertex][index];
}

void Triangle::setVertex(byte vertex, byte index, Triangle *tri) {
  vertices[vertex][index] = tri;
}

RGB *Triangle::getLED(byte vertex) {
  return &(leds[vertex]);
}

void Triangle::setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2) {
  hasLeds = true;
  leds[0].pixel = p0;
  leds[1].pixel = p1;
  leds[2].pixel = p2;
}


void makeEdge(Triangle **triangles, int tri, int edge, int neighbor) {
  triangles[tri]->setEdge(edge, triangles[neighbor]);
}

void makeVertex(Triangle **triangles, int tri, int vertex, int index,
		int neighbor) {
  triangles[tri]->setVertex(vertex, index, triangles[neighbor]);
}

/******************************************************************************
 * Construct an icosohedron
 *
 * Here is the flattened layout:
 *                         ____
 *                       /\    /
 *                      /5 \0 /
 *    ____  ____       /____\/____
 *   \    /\    /\    /\    /\    /
 *    \4 /9 \10/15\  /16\11/6 \1 /
 *     \/____\/____\/____\/____\/
 *           /\    /\    /\
 *          /14\19/18\17/12\
 *         /____\/____\/____\
 *        /\    /\    /\    /\
 *       /3 \8 /  \13/  \7 /2 \
 *      /____\/    \/    \/____\
 */
Triangle** buildIcosohedron() {
  Triangle **triangles = (Triangle **)malloc(sizeof (Triangle *) * 20);
  for (byte i = 0; i < 20; i++) {
    triangles[i] = new Triangle(i);
  }

  // XXX: This is very manual, is there a way to generate this programmatically?
  makeEdge  (triangles,  0,  0,  1);
  makeEdge  (triangles,  0,  1,  5);
  makeEdge  (triangles,  0,  2,  4);
  makeVertex(triangles,  0,  0,  0,  3);
  makeVertex(triangles,  0,  0,  1,  2);
  makeVertex(triangles,  0,  1,  0,  6);
  makeVertex(triangles,  0,  1,  1, 11);
  makeVertex(triangles,  0,  2,  0, 10);
  makeVertex(triangles,  0,  2,  1,  9);

  makeEdge  (triangles,  1,  0,  2);
  makeEdge  (triangles,  1,  1,  6);
  makeEdge  (triangles,  1,  2,  0);
  makeVertex(triangles,  1,  0,  0,  4);
  makeVertex(triangles,  1,  0,  1,  3);
  makeVertex(triangles,  1,  1,  0,  7);
  makeVertex(triangles,  1,  1,  1, 12);
  makeVertex(triangles,  1,  2,  0, 11);
  makeVertex(triangles,  1,  2,  1,  5);

  makeEdge  (triangles,  2,  0,  3);
  makeEdge  (triangles,  2,  1,  7);
  makeEdge  (triangles,  2,  2,  1);
  makeVertex(triangles,  2,  0,  0,  0);
  makeVertex(triangles,  2,  0,  1,  4);
  makeVertex(triangles,  2,  1,  0,  8);
  makeVertex(triangles,  2,  1,  1, 13);
  makeVertex(triangles,  2,  2,  0, 12);
  makeVertex(triangles,  2,  2,  1,  6);

  makeEdge  (triangles,  3,  0,  4);
  makeEdge  (triangles,  3,  1,  8);
  makeEdge  (triangles,  3,  2,  2);
  makeVertex(triangles,  3,  0,  0,  1);
  makeVertex(triangles,  3,  0,  1,  0);
  makeVertex(triangles,  3,  1,  0,  9);
  makeVertex(triangles,  3,  1,  1, 14);
  makeVertex(triangles,  3,  2,  0, 13);
  makeVertex(triangles,  3,  2,  1,  7);

  makeEdge  (triangles,  4,  0,  0);
  makeEdge  (triangles,  4,  1,  9);
  makeEdge  (triangles,  4,  2,  3);
  makeVertex(triangles,  4,  0,  0,  2);
  makeVertex(triangles,  4,  0,  1,  1);
  makeVertex(triangles,  4,  1,  0,  5);
  makeVertex(triangles,  4,  1,  1, 10);
  makeVertex(triangles,  4,  2,  0, 14);
  makeVertex(triangles,  4,  2,  1,  8);

  makeEdge  (triangles,  5,  0, 10);
  makeEdge  (triangles,  5,  1,  0);
  makeEdge  (triangles,  5,  2, 11);
  makeVertex(triangles,  5,  0,  0, 16);
  makeVertex(triangles,  5,  0,  1, 15);
  makeVertex(triangles,  5,  1,  0,  9);
  makeVertex(triangles,  5,  1,  1,  4);
  makeVertex(triangles,  5,  2,  0,  6);
  makeVertex(triangles,  5,  2,  1, 11);

  makeEdge  (triangles,  6,  0, 11);
  makeEdge  (triangles,  6,  1,  1);
  makeEdge  (triangles,  6,  2, 12);
  makeVertex(triangles,  6,  0,  0, 17);
  makeVertex(triangles,  6,  0,  1, 16);
  makeVertex(triangles,  6,  1,  0,  5);
  makeVertex(triangles,  6,  1,  1,  0);
  makeVertex(triangles,  6,  2,  0,  2);
  makeVertex(triangles,  6,  2,  1,  7);

  makeEdge  (triangles,  7,  0, 12);
  makeEdge  (triangles,  7,  1,  2);
  makeEdge  (triangles,  7,  2, 13);
  makeVertex(triangles,  7,  0,  0, 18);
  makeVertex(triangles,  7,  0,  1, 17);
  makeVertex(triangles,  7,  1,  0,  6);
  makeVertex(triangles,  7,  1,  1,  1);
  makeVertex(triangles,  7,  2,  0,  8);
  makeVertex(triangles,  7,  2,  1, 13);

  makeEdge  (triangles,  8,  0, 13);
  makeEdge  (triangles,  8,  1,  3);
  makeEdge  (triangles,  8,  2, 14);
  makeVertex(triangles,  8,  0,  0, 19);
  makeVertex(triangles,  8,  0,  1, 18);
  makeVertex(triangles,  8,  1,  0,  7);
  makeVertex(triangles,  8,  1,  1,  2);
  makeVertex(triangles,  8,  2,  0,  4);
  makeVertex(triangles,  8,  2,  1,  9);

  makeEdge  (triangles,  9,  0,  14);
  makeEdge  (triangles,  9,  1,  4);
  makeEdge  (triangles,  9,  2,  10);
  makeVertex(triangles,  9,  0,  0, 15);
  makeVertex(triangles,  9,  0,  1, 19);
  makeVertex(triangles,  9,  1,  0,  8);
  makeVertex(triangles,  9,  1,  1,  3);
  makeVertex(triangles,  9,  2,  0,  0);
  makeVertex(triangles,  9,  2,  1,  5);

  makeEdge  (triangles, 10,  0,  5);
  makeEdge  (triangles, 10,  1, 15);
  makeEdge  (triangles, 10,  2,  9);
  makeVertex(triangles, 10,  0,  0,  4);
  makeVertex(triangles, 10,  0,  1,  0);
  makeVertex(triangles, 10,  1,  0, 11);
  makeVertex(triangles, 10,  1,  1, 16);
  makeVertex(triangles, 10,  2,  0, 19);
  makeVertex(triangles, 10,  2,  1, 14);

  makeEdge  (triangles, 11,  0,  6);
  makeEdge  (triangles, 11,  1, 16);
  makeEdge  (triangles, 11,  2,  5);
  makeVertex(triangles, 11,  0,  0,  0);
  makeVertex(triangles, 11,  0,  1,  1);
  makeVertex(triangles, 11,  1,  0, 12);
  makeVertex(triangles, 11,  1,  1, 17);
  makeVertex(triangles, 11,  2,  0, 15);
  makeVertex(triangles, 11,  2,  1, 10);

  makeEdge  (triangles, 12,  0,  7);
  makeEdge  (triangles, 12,  1, 17);
  makeEdge  (triangles, 12,  2,  6);
  makeVertex(triangles, 12,  0,  0,  1);
  makeVertex(triangles, 12,  0,  1,  2);
  makeVertex(triangles, 12,  1,  0, 13);
  makeVertex(triangles, 12,  1,  1, 18);
  makeVertex(triangles, 12,  2,  0, 16);
  makeVertex(triangles, 12,  2,  1, 11);

  makeEdge  (triangles, 13,  0,  8);
  makeEdge  (triangles, 13,  1, 18);
  makeEdge  (triangles, 13,  2,  7);
  makeVertex(triangles, 13,  0,  0,  2);
  makeVertex(triangles, 13,  0,  1,  3);
  makeVertex(triangles, 13,  1,  0, 14);
  makeVertex(triangles, 13,  1,  1, 19);
  makeVertex(triangles, 13,  2,  0, 17);
  makeVertex(triangles, 13,  2,  1, 12);

  makeEdge  (triangles, 14,  0,  9);
  makeEdge  (triangles, 14,  1, 19);
  makeEdge  (triangles, 14,  2,  8);
  makeVertex(triangles, 14,  0,  0,  3);
  makeVertex(triangles, 14,  0,  1,  4);
  makeVertex(triangles, 14,  1,  0, 10);
  makeVertex(triangles, 14,  1,  1, 15);
  makeVertex(triangles, 14,  2,  0, 18);
  makeVertex(triangles, 14,  2,  1, 13);

  makeEdge  (triangles, 15,  0, 19);
  makeEdge  (triangles, 15,  1, 10);
  makeEdge  (triangles, 15,  2, 16);
  makeVertex(triangles, 15,  0,  0, 17);
  makeVertex(triangles, 15,  0,  1, 18);
  makeVertex(triangles, 15,  1,  0, 14);
  makeVertex(triangles, 15,  1,  1,  9);
  makeVertex(triangles, 15,  2,  0,  5);
  makeVertex(triangles, 15,  2,  1, 11);

  makeEdge  (triangles, 16,  0, 15);
  makeEdge  (triangles, 16,  1, 11);
  makeEdge  (triangles, 16,  2, 17);
  makeVertex(triangles, 16,  0,  0, 18);
  makeVertex(triangles, 16,  0,  1, 19);
  makeVertex(triangles, 16,  1,  0, 10);
  makeVertex(triangles, 16,  1,  1,  5);
  makeVertex(triangles, 16,  2,  0,  6);
  makeVertex(triangles, 16,  2,  1,  12);

  makeEdge  (triangles, 17,  0, 16);
  makeEdge  (triangles, 17,  1, 12);
  makeEdge  (triangles, 17,  2, 18);
  makeVertex(triangles, 17,  0,  0, 19);
  makeVertex(triangles, 17,  0,  1, 15);
  makeVertex(triangles, 17,  1,  0, 11);
  makeVertex(triangles, 17,  1,  1,  6);
  makeVertex(triangles, 17,  2,  0,  7);
  makeVertex(triangles, 17,  2,  1, 13);

  makeEdge  (triangles, 18,  0, 17);
  makeEdge  (triangles, 18,  1, 13);
  makeEdge  (triangles, 18,  2, 19);
  makeVertex(triangles, 18,  0,  0, 15);
  makeVertex(triangles, 18,  0,  1, 16);
  makeVertex(triangles, 18,  1,  0, 12);
  makeVertex(triangles, 18,  1,  1, 17);
  makeVertex(triangles, 18,  2,  0,  8);
  makeVertex(triangles, 18,  2,  1, 14);

  makeEdge  (triangles, 19,  0, 18);
  makeEdge  (triangles, 19,  1, 14);
  makeEdge  (triangles, 19,  2, 15);
  makeVertex(triangles, 19,  0,  0, 16);
  makeVertex(triangles, 19,  0,  1, 17);
  makeVertex(triangles, 19,  1,  0, 13);
  makeVertex(triangles, 19,  1,  1,  8);
  makeVertex(triangles, 19,  2,  0,  9);
  makeVertex(triangles, 19,  2,  1, 10);

  return triangles;
}
