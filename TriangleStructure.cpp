#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_MID
#include "Debug.h"

#include "PixelUtil.h"
#include "TriangleStructure.h"

Triangle::Triangle(unsigned int _id) {
  hasLeds = false;
  updated = false;
  mark = 0;
  id = _id;

  for (int e = 0; e < TRIANGLE_NUM_EDGES; e++) {
    edges[e] = NULL;
  }

  for (int v = 0; v < TRIANGLE_NUM_VERTICES; v++) {
    for (int o = 0; o < TRIANGLE_VERTEX_ORDER; o++) {
      vertices[v][o] = NULL;
    }
  }

  DEBUG_VALUELN(DEBUG_MID, "Created Triangle ", id);
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

void Triangle::setColor(byte r, byte g, byte b) {
  if (hasLeds) {
    for (int i = 0; i < 3; i++) {
      leds[i].setColor(r, g, b);
    }
    updated = true;
  }
}

void Triangle::setColor(byte led, byte r, byte g, byte b) {
  if (hasLeds) {
    leds[led].setColor(r, g, b);
    updated = true;
  }
}

void Triangle::setColor(uint32_t c) {
  if (hasLeds) {
    for (int i = 0; i < 3; i++) {
      leds[i].setColor(c);
    }
    updated = true;
  }
}

void Triangle::setColor(byte led, uint32_t c) {
  if (hasLeds) {
    leds[led].setColor(c);
    updated = true;
  }
}

uint32_t Triangle::getColor() {
  if (hasLeds) {
    return leds[0].color();
  } else {
    return 0;
  }
}

uint32_t Triangle::getColor(byte led) {
  if (hasLeds) {
    return leds[led].color();
  } else {
    return 0;
  }
}

byte Triangle::getRed() {
  if (hasLeds) {
    return leds[0].red();
  } else {
    return 0;
  }
}

byte Triangle::getRed(byte vertex) {
  if (hasLeds) {
    return leds[vertex].red();
  } else {
    return 0;
  }
}


byte Triangle::getGreen() {
  if (hasLeds) {
    return leds[0].green();
  } else {
    return 0;
  }
}

byte Triangle::getGreen(byte vertex) {
  if (hasLeds) {
    return leds[vertex].green();
  } else {
    return 0;
  }
}


byte Triangle::getBlue() {
  if (hasLeds) {
    return leds[0].blue();
  } else {
    return 0;
  }
}

byte Triangle::getBlue(byte vertex) {
  if (hasLeds) {
    return leds[vertex].blue();
  } else {
    return 0;
  }
}

/*
 * This is used to find the LED adjacent to a corner triangle
 *      S
 *     \/
 *     ^
 *    /?\
 *   /   \
 *  /     \
 * /?_____?\
 *
 */
byte Triangle::matchVertex(Triangle *neighbor) {
  for (byte v = 0; v < TRIANGLE_NUM_VERTICES; v++) {
    for (byte i = 0; i < TRIANGLE_VERTEX_ORDER; i++) {
      if (vertices[v][i] == neighbor) {
	return v;
      }
    }
  }
  return 255;
}

/*
 * Return the vertex that lies left of the indicated triangle
 *     ^--------
 *    /?\V     /
 *   /   \ N  /
 *  /     \  /
 * /_______\/
 */
byte Triangle::matchVertexRight(Triangle *neighbor, byte vertex) {
  /* Find the edge of the neighbor */
  for (byte edge = 0; edge < TRIANGLE_NUM_EDGES; edge++) {
    if (edges[edge] == neighbor) {
      return edge;
    }
  }
  return (byte)-1;
}

/*
 * Return the vertex that lies right of the indicated triangle
 * --------^
 * \     V/?\
 *  \  N /   \
 *   \  /     \
 *    \/_______\
 */
byte Triangle::matchVertexLeft(Triangle *neighbor, byte vertex) {
  /* Find the edge of the neighbor */
  for (byte edge = 0; edge < TRIANGLE_NUM_EDGES; edge++) {
    if (edges[edge] == neighbor) {
      return (edge + 2) % TRIANGLE_NUM_EDGES;
    }
  }
  return (byte)-1;
}

/*
 * Return the triangle to the left of the indicated vertex
 *-----^
 *    /V\
 * X /   \
 *  /     \
 * /_______\
 */
Triangle *Triangle::leftOfVertex(byte vertex) {
  switch (vertex) {
  case 0: return edges[2]; break;
  case 1: return edges[0]; break;
  case 2: return edges[1]; break;
  default:
    DEBUG_ERR(F("leftOfVertex: invalid vertex"));
    return NULL;
    break;
  }
}

/*
 * Return the triangle to the right of the indicated vertex
 *     ^
 *    /V\
 *   /   \X
 *  /     \
 * /_______\
 */
Triangle *Triangle::rightOfVertex(byte vertex) {
  return edges[vertex];
}

void Triangle::print(byte level) {
  DEBUG_VALUE(level, "Tri: ", id);
  for (int e = 0; e < TRIANGLE_NUM_EDGES; e++) {
    if (edges[e] != NULL) DEBUG_VALUE(level, " e:", edges[e]->id);
  }

  for (int v = 0; v < TRIANGLE_NUM_VERTICES; v++) {
    DEBUG_VALUE(level, " v:", v);
    for (int o = 0; o < TRIANGLE_VERTEX_ORDER; o++) {
      if (vertices[v][o] != NULL) DEBUG_VALUE(level, " ", vertices[v][o]->id);
    }
  }

  DEBUG_PRINTLN(level, "");
}


/******************************************************************************
 * Topology construction helper functions
 */

void makeEdge(Triangle *triangles, int tri, int edge, int neighbor) {
  triangles[tri].setEdge(edge, &triangles[neighbor]);
}

void makeVertex(Triangle *triangles, int tri, int vertex, int index,
		int neighbor) {
  triangles[tri].setVertex(vertex, index, &triangles[neighbor]);
}

void setLeds(Triangle *triangles, int tri, int baseLed) {
  triangles[tri].setLedPixels(baseLed, baseLed - 1, baseLed - 2);
}

void setLeds(Triangle *triangles, int tri, int led1, int led2, int led3) {
  triangles[tri].setLedPixels(led1, led2, led3);
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
Triangle triangleArray[20];
Triangle* buildIcosohedron(int *numTriangles, int numLeds) {
  int triangleCount = 20;

  Triangle *triangles = &(triangleArray[0]);//(Triangle *)malloc(sizeof (Triangle) * triangleCount);
  DEBUG_COMMAND(DEBUG_ERROR,
		if (triangles == NULL) {
		  DEBUG_ERR(F("Failed to malloc triangles"));
		  debug_err_state(DEBUG_ERR_MALLOC);
		}
		);

  for (byte i = 0; i < triangleCount; i++) {
    triangles[i] = Triangle(i);
  }

  int led = numLeds - 1;
#if 0 // This is the HTML prototype module config
  setLeds(triangles, 0, led); led -= 3; // 47 - R
  setLeds(triangles, 4, led); led -= 3; // 44 - G
  setLeds(triangles, 3, led); led -= 3; // 41 - B
  setLeds(triangles, 2, led); led -= 3; // 38 - Y
  setLeds(triangles, 1, led); led -= 3; // 35 - P
  setLeds(triangles, 6, led); led -= 3; // 32 - T,R

  setLeds(triangles, 12, led); led -= 3; // 29 - G
  setLeds(triangles, 7,  led); led -= 3; // 26 - B
  setLeds(triangles, 13, led); led -= 3; // 23 - Y
  setLeds(triangles, 18, led); led -= 3; // 20 - P
  setLeds(triangles, 19, led); led -= 3; // 17 - T,R

  setLeds(triangles, 8,  led); led -= 3; // 14 - G
  setLeds(triangles, 14, led); led -= 3; // 11 - B
  setLeds(triangles, 9,  led); led -= 3; // 8 - Y
  setLeds(triangles, 10, led); led -= 3; // 5 - P
  setLeds(triangles, 5,  led); led -= 3; // 2 - T,R
#endif
#if 1
  setLeds(triangles, 0,  led, led-1, led-2); led -= 3; // 47 - R x
  setLeds(triangles, 5,  led-2, led, led-1); led -= 3; // 44 - G x
  setLeds(triangles, 11, led-1, led-2, led); led -= 3; // 41 - B x
  setLeds(triangles, 6,  led, led-1, led-2); led -= 3; // 38 - Y x
  setLeds(triangles, 1,  led-2, led, led-1); led -= 3; // 35 - P x
  setLeds(triangles, 3,  led, led-2, led-1); led -= 3; // 32 - T,R x

  setLeds(triangles, 4,  led-1, led-2, led); led -= 3; // 29 - G x
  setLeds(triangles, 9,  led-2, led-1, led); led -= 3; // 26 - B x
  setLeds(triangles, 10, led-1, led-2, led); led -= 3; // 23 - Y x
  setLeds(triangles, 15, led-2, led-1, led); led -= 3; // 20 - P x ??
  setLeds(triangles, 16, led, led-1, led-2); led -= 3; // 17 - T,R x

  setLeds(triangles, 17, led-1, led, led-2); led -= 3; // 14 - G x
  setLeds(triangles, 12, led-2, led, led-1); led -= 3; // 11 - B x
  setLeds(triangles, 2,  led-1, led-2, led); led -= 3; // 8 - Y x
  led--;   // Had to skip an LED due to wire lengths
  setLeds(triangles, 14, led, led-2, led-1); led -= 3; // 4 - P x
  setLeds(triangles, 19, led-2, led-1, led); led -= 3; // 1 - T,R

#endif
  // XXX: This is very manual, is there a way to generate this programmatically?
  *numTriangles = 0;
  makeEdge  (triangles,  0,  0,  1);
  makeEdge  (triangles,  0,  1,  5);
  makeEdge  (triangles,  0,  2,  4);
  makeVertex(triangles,  0,  0,  0,  3);
  makeVertex(triangles,  0,  0,  1,  2);
  makeVertex(triangles,  0,  1,  0,  6);
  makeVertex(triangles,  0,  1,  1, 11);
  makeVertex(triangles,  0,  2,  0, 10);
  makeVertex(triangles,  0,  2,  1,  9);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  1,  0,  2);
  makeEdge  (triangles,  1,  1,  6);
  makeEdge  (triangles,  1,  2,  0);
  makeVertex(triangles,  1,  0,  0,  4);
  makeVertex(triangles,  1,  0,  1,  3);
  makeVertex(triangles,  1,  1,  0,  7);
  makeVertex(triangles,  1,  1,  1, 12);
  makeVertex(triangles,  1,  2,  0, 11);
  makeVertex(triangles,  1,  2,  1,  5);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  2,  0,  3);
  makeEdge  (triangles,  2,  1,  7);
  makeEdge  (triangles,  2,  2,  1);
  makeVertex(triangles,  2,  0,  0,  0);
  makeVertex(triangles,  2,  0,  1,  4);
  makeVertex(triangles,  2,  1,  0,  8);
  makeVertex(triangles,  2,  1,  1, 13);
  makeVertex(triangles,  2,  2,  0, 12);
  makeVertex(triangles,  2,  2,  1,  6);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  3,  0,  4);
  makeEdge  (triangles,  3,  1,  8);
  makeEdge  (triangles,  3,  2,  2);
  makeVertex(triangles,  3,  0,  0,  1);
  makeVertex(triangles,  3,  0,  1,  0);
  makeVertex(triangles,  3,  1,  0,  9);
  makeVertex(triangles,  3,  1,  1, 14);
  makeVertex(triangles,  3,  2,  0, 13);
  makeVertex(triangles,  3,  2,  1,  7);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  4,  0,  0);
  makeEdge  (triangles,  4,  1,  9);
  makeEdge  (triangles,  4,  2,  3);
  makeVertex(triangles,  4,  0,  0,  2);
  makeVertex(triangles,  4,  0,  1,  1);
  makeVertex(triangles,  4,  1,  0,  5);
  makeVertex(triangles,  4,  1,  1, 10);
  makeVertex(triangles,  4,  2,  0, 14);
  makeVertex(triangles,  4,  2,  1,  8);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  5,  0, 10);
  makeEdge  (triangles,  5,  1,  0);
  makeEdge  (triangles,  5,  2, 11);
  makeVertex(triangles,  5,  0,  0, 16);
  makeVertex(triangles,  5,  0,  1, 15);
  makeVertex(triangles,  5,  1,  0,  9);
  makeVertex(triangles,  5,  1,  1,  4);
  makeVertex(triangles,  5,  2,  0,  1);
  makeVertex(triangles,  5,  2,  1,  6);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  6,  0, 11);
  makeEdge  (triangles,  6,  1,  1);
  makeEdge  (triangles,  6,  2, 12);
  makeVertex(triangles,  6,  0,  0, 17);
  makeVertex(triangles,  6,  0,  1, 16);
  makeVertex(triangles,  6,  1,  0,  5);
  makeVertex(triangles,  6,  1,  1,  0);
  makeVertex(triangles,  6,  2,  0,  2);
  makeVertex(triangles,  6,  2,  1,  7);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  7,  0, 12);
  makeEdge  (triangles,  7,  1,  2);
  makeEdge  (triangles,  7,  2, 13);
  makeVertex(triangles,  7,  0,  0, 18);
  makeVertex(triangles,  7,  0,  1, 17);
  makeVertex(triangles,  7,  1,  0,  6);
  makeVertex(triangles,  7,  1,  1,  1);
  makeVertex(triangles,  7,  2,  0,  8);
  makeVertex(triangles,  7,  2,  1, 13);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  8,  0, 13);
  makeEdge  (triangles,  8,  1,  3);
  makeEdge  (triangles,  8,  2, 14);
  makeVertex(triangles,  8,  0,  0, 19);
  makeVertex(triangles,  8,  0,  1, 18);
  makeVertex(triangles,  8,  1,  0,  7);
  makeVertex(triangles,  8,  1,  1,  2);
  makeVertex(triangles,  8,  2,  0,  4);
  makeVertex(triangles,  8,  2,  1,  9);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles,  9,  0,  14);
  makeEdge  (triangles,  9,  1,  4);
  makeEdge  (triangles,  9,  2,  10);
  makeVertex(triangles,  9,  0,  0, 15);
  makeVertex(triangles,  9,  0,  1, 19);
  makeVertex(triangles,  9,  1,  0,  8);
  makeVertex(triangles,  9,  1,  1,  3);
  makeVertex(triangles,  9,  2,  0,  0);
  makeVertex(triangles,  9,  2,  1,  5);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 10,  0,  5);
  makeEdge  (triangles, 10,  1, 15);
  makeEdge  (triangles, 10,  2,  9);
  makeVertex(triangles, 10,  0,  0,  4);
  makeVertex(triangles, 10,  0,  1,  0);
  makeVertex(triangles, 10,  1,  0, 11);
  makeVertex(triangles, 10,  1,  1, 16);
  makeVertex(triangles, 10,  2,  0, 19);
  makeVertex(triangles, 10,  2,  1, 14);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 11,  0,  6);
  makeEdge  (triangles, 11,  1, 16);
  makeEdge  (triangles, 11,  2,  5);
  makeVertex(triangles, 11,  0,  0,  0);
  makeVertex(triangles, 11,  0,  1,  1);
  makeVertex(triangles, 11,  1,  0, 12);
  makeVertex(triangles, 11,  1,  1, 17);
  makeVertex(triangles, 11,  2,  0, 15);
  makeVertex(triangles, 11,  2,  1, 10);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 12,  0,  7);
  makeEdge  (triangles, 12,  1, 17);
  makeEdge  (triangles, 12,  2,  6);
  makeVertex(triangles, 12,  0,  0,  1);
  makeVertex(triangles, 12,  0,  1,  2);
  makeVertex(triangles, 12,  1,  0, 13);
  makeVertex(triangles, 12,  1,  1, 18);
  makeVertex(triangles, 12,  2,  0, 16);
  makeVertex(triangles, 12,  2,  1, 11);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 13,  0,  8);
  makeEdge  (triangles, 13,  1, 18);
  makeEdge  (triangles, 13,  2,  7);
  makeVertex(triangles, 13,  0,  0,  2);
  makeVertex(triangles, 13,  0,  1,  3);
  makeVertex(triangles, 13,  1,  0, 14);
  makeVertex(triangles, 13,  1,  1, 19);
  makeVertex(triangles, 13,  2,  0, 17);
  makeVertex(triangles, 13,  2,  1, 12);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 14,  0,  9);
  makeEdge  (triangles, 14,  1, 19);
  makeEdge  (triangles, 14,  2,  8);
  makeVertex(triangles, 14,  0,  0,  3);
  makeVertex(triangles, 14,  0,  1,  4);
  makeVertex(triangles, 14,  1,  0, 10);
  makeVertex(triangles, 14,  1,  1, 15);
  makeVertex(triangles, 14,  2,  0, 18);
  makeVertex(triangles, 14,  2,  1, 13);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 15,  0, 19);
  makeEdge  (triangles, 15,  1, 10);
  makeEdge  (triangles, 15,  2, 16);
  makeVertex(triangles, 15,  0,  0, 17);
  makeVertex(triangles, 15,  0,  1, 18);
  makeVertex(triangles, 15,  1,  0, 14);
  makeVertex(triangles, 15,  1,  1,  9);
  makeVertex(triangles, 15,  2,  0,  5);
  makeVertex(triangles, 15,  2,  1, 11);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 16,  0, 15);
  makeEdge  (triangles, 16,  1, 11);
  makeEdge  (triangles, 16,  2, 17);
  makeVertex(triangles, 16,  0,  0, 18);
  makeVertex(triangles, 16,  0,  1, 19);
  makeVertex(triangles, 16,  1,  0, 10);
  makeVertex(triangles, 16,  1,  1,  5);
  makeVertex(triangles, 16,  2,  0,  6);
  makeVertex(triangles, 16,  2,  1,  12);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 17,  0, 16);
  makeEdge  (triangles, 17,  1, 12);
  makeEdge  (triangles, 17,  2, 18);
  makeVertex(triangles, 17,  0,  0, 19);
  makeVertex(triangles, 17,  0,  1, 15);
  makeVertex(triangles, 17,  1,  0, 11);
  makeVertex(triangles, 17,  1,  1,  6);
  makeVertex(triangles, 17,  2,  0,  7);
  makeVertex(triangles, 17,  2,  1, 13);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 18,  0, 17);
  makeEdge  (triangles, 18,  1, 13);
  makeEdge  (triangles, 18,  2, 19);
  makeVertex(triangles, 18,  0,  0, 15);
  makeVertex(triangles, 18,  0,  1, 16);
  makeVertex(triangles, 18,  1,  0, 12);
  makeVertex(triangles, 18,  1,  1,  7);
  makeVertex(triangles, 18,  2,  0,  8);
  makeVertex(triangles, 18,  2,  1, 14);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

  makeEdge  (triangles, 19,  0, 18);
  makeEdge  (triangles, 19,  1, 14);
  makeEdge  (triangles, 19,  2, 15);
  makeVertex(triangles, 19,  0,  0, 16);
  makeVertex(triangles, 19,  0,  1, 17);
  makeVertex(triangles, 19,  1,  0, 13);
  makeVertex(triangles, 19,  1,  1,  8);
  makeVertex(triangles, 19,  2,  0,  9);
  makeVertex(triangles, 19,  2,  1, 10);
  (*numTriangles)++;
  if (*numTriangles == triangleCount) goto ICOS_DONE;

 ICOS_DONE:
  DEBUG_COMMAND(DEBUG_MID,
		for (int t = 0; t < *numTriangles; t++) {
		  triangles[t].print(DEBUG_MID);
		}
		);

  DEBUG_VALUELN(DEBUG_MID, "Icos numTriangles:", *numTriangles);
  return triangles;
}

/* Send updated values to a Pixel chain */
void updateTrianglePixels(Triangle *triangles, int numTriangles,
			  PixelUtil *pixels) {
  boolean update = false;
  int updated = 0;
  for (int tri = 0; tri < numTriangles; tri++) {
    if (triangles[tri].updated) {
      update = true;
      for (byte led = 0; led < 3; led++) {
	pixels->setPixelRGB(&(triangles[tri].leds[led]));
      }
      triangles[tri].updated = false;
      DEBUG_VALUE(DEBUG_HIGH, "XXX: TRIANGLES UPDATED: addr=", (int)&triangles[tri]);
      updated++;
    }
  }

  if (update) {
    pixels->update();
    DEBUG_VALUELN(DEBUG_HIGH, "Updated triangles:", updated);
  }
}
