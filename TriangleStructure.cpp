#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleStructure.h"

Triangle *Triangle::getEdge(byte edge) {
  return edges[edge];
}

void Triangle::setEdge(byte edge, Triangle *tri) {
  edges[edge] = tri;
}

Triangle *Triangle::getVertex(byte vertex) {
  return vertices[vertex];
}

void Triangle::setVertex(byte vertex, Triangle *tri) {
  vertices[vertex] = tri;
}
