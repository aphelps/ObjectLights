#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_LOW
#include "Debug.h"

#include "EEPromUtils.h"

#include "PixelUtil.h"
#include "TriangleStructure.h"

// XXX: Relying on an array allocated by the main sketch feels icky
extern Triangle *triangles;

Triangle::Triangle(unsigned int _id) {
  updated = false;
  mark = 0;
  id = _id;

  for (int e = 0; e < NUM_EDGES; e++) {
    edges[e] = NO_ID;
  }

  for (int v = 0; v < NUM_VERTICES; v++) {
    leds[v].pixel = NO_LED;
  }

  DEBUG3_VALUELN("Created Triangle id:", id);
}

Triangle *Triangle::getEdge(byte edge) {
  if (edges[edge] == NO_EDGE) 
    return NULL;
  return &triangles[edges[edge]];
}

void Triangle::setEdge(byte edge, Triangle *tri) {
  edges[edge] = tri->id;
}

void  Triangle::setEdge(byte edge, byte neighbor) {
  edges[edge] = neighbor;
}

/*
 * Get the vertex neighbor by traversing clockwise.
 *
 *      ______
 *    /\      /
 *   /  \  1 /
 *  /  0 \  /
 * /______\/______
 * \     c/\      /
 *  \  L /V \ E  /
 *   \  /    \  /
 *    \/______\/
 */
Triangle *Triangle::getVertex(byte vertex, geo_id_t index) {
  Triangle *neighbor = leftOfVertex(vertex);  // L in diagram
  Triangle *end = rightOfVertex(vertex); // E in diangram
  Triangle *current = this;
  byte currentVertex = vertex; // V in diagran
  
  for (byte i = 0; i <= index; i++) {
    if (neighbor == NULL) return NULL;

    currentVertex = neighbor->matchVertexRight(current, 
                                               currentVertex); // c in diagram
    if (currentVertex == NO_VERTEX) return NULL;

    current = neighbor;
    neighbor = neighbor->leftOfVertex(currentVertex);

    if (neighbor == end) return NULL;
  }

  return neighbor;
}

geo_id_t Triangle::getVertexID(byte vertex, byte index) {
  Triangle *neighbor = getVertex(vertex, index);
  if (neighbor) return neighbor->id;
  else return NO_ID;
}

PRGB *Triangle::getLED(byte vertex) {
  return &(leds[vertex]);
}

void Triangle::setLedPixel(byte led, uint16_t pixel) {
  leds[led].pixel = pixel;
}

void Triangle::setLedPixels(uint16_t p0, uint16_t p1, uint16_t p2) {
  leds[0].pixel = p0;
  leds[1].pixel = p1;
  leds[2].pixel = p2;
}

void Triangle::setColor(byte r, byte g, byte b) {
  if (hasLeds()) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setColor(r, g, b);
    }
    updated = true;
  }
}



void Triangle::setColor(byte led, byte r, byte g, byte b) {
  if (hasLeds()) {
    leds[led].setColor(r, g, b);
    updated = true;
  }
}

void Triangle::setColor(uint32_t c) {
  if (hasLeds()) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].setColor(c);
    }
    updated = true;
  }
}

void Triangle::setColor(byte led, uint32_t c) {
  if (hasLeds()) {
    leds[led].setColor(c);
    updated = true;
  }
}

extern PixelUtil pixels;

uint32_t Triangle::getColor() {
  if (hasLeds()) {
    return pixels.getColor(leds[0].pixel);
  } else {
    return 0;
  }
}

uint32_t Triangle::getColor(byte led) {
  if (hasLeds()) {
    return pixels.getColor(leds[led].pixel);
  } else {
    return 0;
  }
}

byte Triangle::getRed() { // TODO: This stuff can probably be moved into Geometry
  if (hasLeds()) {
    return pixel_red(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Triangle::getRed(byte vertex) {
  if (hasLeds()) {
    return pixel_red(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}


byte Triangle::getGreen() {
  if (hasLeds()) {
    return pixel_green(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Triangle::getGreen(byte vertex) {
  if (hasLeds()) {
    return pixel_green(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}


byte Triangle::getBlue() {
  if (hasLeds()) {
    return pixel_blue(pixels.getColor(leds[0].pixel));
  } else {
    return 0;
  }
}

byte Triangle::getBlue(byte vertex) {
  if (hasLeds()) {
    return pixel_blue(pixels.getColor(leds[vertex].pixel));
  } else {
    return 0;
  }
}

/******************************************************************************
 * Topology traversal functions
 */

/*
 * This is used to find the LED adjacent to a corner triangle
 *     N
 *     \/
 *     ^
 *    /?\
 *   /   \
 *  /     \
 * /_______\
 *
 */
byte Triangle::matchVertex(Triangle *neighbor) {
  for (byte v = 0; v < NUM_VERTICES; v++) {
    for (byte i = 0; i < VERTEX_ORDER; i++) {
      if (getVertex(v, i) == neighbor) {
	return v;
      }
    }
  }
  return NO_ID;
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
  for (byte edge = 0; edge < NUM_EDGES; edge++) {
    if (getEdge(edge) == neighbor) {
      // XXX - This doesn't consider the vertex at all!!!
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
  for (byte edge = 0; edge < NUM_EDGES; edge++) {
    if (getEdge(edge) == neighbor) {
      // XXX - This doesn't consider the vertex at all!!!
      return (edge + 1) % NUM_EDGES;
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
  case 0: return getEdge(2); break;
  case 1: return getEdge(0); break;
  case 2: return getEdge(1); break;
  default:
    DEBUG1_VALUE("leftOfVertex: invalid vertex:", vertex);
    DEBUG1_VALUELN(" id:", id);
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
  if (vertex >= NUM_VERTICES) {
    DEBUG1_VALUE("rightOfVertex: invalid vertex:", vertex);
    DEBUG1_VALUELN(" id:", id);
    return NULL;
  }
  return getEdge(vertex);
}

/*
 * Print a representation of the triangle
 */
void Triangle::print() {
  DEBUG2_VALUE(" Tri:", id);
  for (int e = 0; e < NUM_EDGES; e++) {
    DEBUG2_VALUE("\te:", edges[e]);
  }

  for (int v = 0; v < NUM_VERTICES; v++) {
    DEBUG2_VALUE("\tv:", leds[v].pixel);

    for (int o = 0; o < VERTEX_ORDER; o++) {
      DEBUG2_VALUE("-", getVertexID(v, o));
    }
  }

  DEBUG2_PRINTLN("");
}


/******************************************************************************
 * Topology construction helper functions
 */

void makeEdge(Triangle *triangles, int tri, int edge, int neighbor) {
  triangles[tri].setEdge(edge, &triangles[neighbor]);
}

void makeVertex(Triangle *triangles, int tri, int vertex, int index,
		int neighbor) {
  //XXX: Ditch this? triangles[tri].setVertex(vertex, index, &triangles[neighbor]);
}

void setLeds(Triangle *triangles, int tri, int baseLed) {
  triangles[tri].setLedPixels(baseLed, baseLed - 1, baseLed - 2);
}

void setLeds(Triangle *triangles, int tri, int led1, int led2, int led3) {
  triangles[tri].setLedPixels(led1, led2, led3);
}

/* Initialize the triangle array */
Triangle triangleArray[TRI_ARRAY_SIZE];
Triangle* initTriangles(int triangleCount) {
  if (triangleCount > TRI_ARRAY_SIZE) {
    DEBUG_ERR("initTriangles: Too many triangles specified");
    DEBUG_ERR_STATE(13);
  }

  Triangle *newtriangles = &(triangleArray[0]);//(Triangle *)malloc(sizeof (Triangle) * triangleCount);
  DEBUG1_COMMAND(
                if (newtriangles == NULL) {
                  DEBUG_ERR("Failed to malloc triangles");
                  debug_err_state(DEBUG_ERR_MALLOC);
                }
                );

  for (byte i = 0; i < triangleCount; i++) {
    newtriangles[i] = Triangle(i);
  }

  return newtriangles;
}

/******************************************************************************
 * Construct a cylinder
 *
 *
 *    ____  ____  ____  ____  _____
 *   \    /\    /\    /\    /\    /\
 *    \0 /1 \2 /3 \4 /5 \6 /7 \8 /9 \
 *     \/____\/____\/____\/____\/____\
 *      \    /\    /\    /\    /\    /\
 *       \10/11\12/13\14/15\16/17\18/19\
 *        \/____\/____\/____\/____\/____\
 *         \    /\    /\    /\    /\    /\
 *          \20/21\22/23\24/25\26/27\28/29\
 *           \/____\/____\/____\/____\/____\
 */
Triangle* buildCylinder(int *numTriangles, int numLeds) {
  int triangleCount = 30;

  triangles = initTriangles(triangleCount);

  int tri;
  for (tri = 0; tri < triangleCount; tri++) {
    if (tri * 3 + 2 < numLeds)
      setLeds(triangles, tri, tri * 3, tri * 3 + 1, tri * 3 + 2);

    if (tri % 2 == 0) {
      if (tri >= 9) makeEdge  (triangles,  tri,  0,  tri - 9);
      makeEdge  (triangles,  tri,  1,  
		 (tri - (tri % 10)) + ((tri + 1) % 10));
      makeEdge  (triangles,  tri,  2,  
		 (tri - (tri % 10)) + ((tri + -1 + 10) % 10));

      makeVertex(triangles,  tri,  0,  0,  
		 (tri - (tri % 10)) + ((tri -2 + 10 ) % 10));
      if (tri >= 10) makeVertex(triangles,  tri,  0,  1,  tri - 10);

      if (tri >= 8) makeVertex(triangles,  tri,  1,  0,  tri - 8 );
      makeVertex(triangles,  tri,  1,  1, 
		 (tri - (tri % 10)) + ((tri + 2) % 10));

      if (tri + 10 < triangleCount) 
	makeVertex(triangles,  tri,  2,  0, tri + 10);
      if (tri + 8 < triangleCount) 
	makeVertex(triangles,  tri,  2,  1,  tri + 8);
    } else {
      makeEdge  (triangles,  tri,  0, (tri - (tri % 10)) + (tri + 1 % 10));
      if (tri + 9 < triangleCount)  makeEdge  (triangles,  tri,  1, tri + 9);
      makeEdge  (triangles,  tri,  2,  (tri - (tri % 10)) + ((tri + -1 + 10) % 10));

      if (tri >= 10) makeVertex(triangles,  tri,  0,  0,  tri - 10);
      if (tri >= 8)  makeVertex(triangles,  tri,  0,  1,  tri - 8);

      makeVertex(triangles,  tri,  1,  0, (tri - (tri % 10)) + ((tri + 2) % 10));
      if (tri + 10 < triangleCount) makeVertex(triangles,  tri,  1,  1, tri + 10);

      if (tri + 8 < triangleCount) makeVertex(triangles,  tri,  2,  0, tri + 8);
      makeVertex(triangles,  tri,  2,  1, (tri - (tri % 10)) + ((tri + -2 + 10) % 10));
    }
  }
  *numTriangles = tri;

  DEBUG3_VALUELN("Cylinder numTriangles:", *numTriangles);

  return triangles;
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
Triangle* buildIcosohedron(int *numTriangles, int numLeds) {
  int triangleCount = 20;

  triangles = initTriangles(triangleCount);

  int led = numLeds - 1;
#define TRI_VERSION 2
#if TRI_VERSION == 0 // This is the HTML prototype module config
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
#if TRI_VERSION == 1
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

#if TRI_VERSION == 2 // This is the HTML prototype module config
  for (int i = 0; i < triangleCount; i++) {
    setLeds(triangles, i, led); led -= 3;
  }
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
  DEBUG3_COMMAND(
		for (int t = 0; t < *numTriangles; t++) {
		  triangles[t].print();
		}
		);

  DEBUG3_VALUELN("Icos numTriangles:", *numTriangles);
  return triangles;
}

/******************************************************************************
 * Serialization functions
 *
 * Serialized format:
 *   ID
 *   IDs of edges
 *   offset of pixels
 * 
 */
int Triangle::toBytes(byte *bytes, int size) {
  triangle_config_t *config = (triangle_config_t *)bytes;
  config->id = id;  

  // Write out the IDs of the adjacent edges
  for (int face = 0; face < NUM_EDGES; face++) {
    config->edges[face] = edges[face];
  }
  
  // Write out the pixel values
  for (int led = 0; led < NUM_LEDS; led++ ) {
    config->leds[led] = leds[led].pixel;
  }

  return sizeof (triangle_config_t);
}

void Triangle::fromBytes(byte *bytes, int size, Geometry *triangles, 
                         geo_id_t numTriangles) {
  triangle_config_t *config = (triangle_config_t *)bytes;

  // Read the ID
  id = config->id;

  // Read the IDs of the adjacent edges
  for (int face = 0; face < NUM_EDGES; face++) {
    edges[face] = config->edges[face];
  }

  // Read the pixel values
  for (int led = 0; led < NUM_LEDS; led++ ) {
    leds[led].pixel = config->leds[led];
  }
}

// This should be the max of triangle_config_t and geometry_config_t
#define MAX_CONFIG_SZ (sizeof (geometry_config_t) + 1)

int readTriangleStructure(int offset, Triangle **triangles_ptr, 
                          int *numTriangles) {
  byte bytes[MAX_CONFIG_SZ];
  
  int newoffset = EEPROM_safe_read(offset, bytes, MAX_CONFIG_SZ);
  if (newoffset - offset != EEPROM_SIZE(sizeof (geometry_config_t))) {
    DEBUG1_VALUELN("Initial size invalid:", newoffset - offset);
    return -1;
  }
  offset = newoffset;

  geometry_config_t *config = (geometry_config_t *)bytes;
  DEBUG2_VALUE("Read triangles offset=", offset);
  DEBUG2_VALUE(" version=", config->version);
  DEBUG2_VALUELN(" num=", config->num_objects);

  // Copy relevant data from config before next read
  uint16_t readTriangles = config->num_objects;

  // TODO: Triangles should be allocated here.
  //Triangle *triangles = (Triangle *)calloc(config->num_objects,
  //                                         sizeof (Triangle));
  Triangle *triangles = initTriangles(readTriangles);

  for (int face = 0; face < readTriangles; face++) {
    offset = EEPROM_safe_read(offset, bytes, MAX_CONFIG_SZ);
    triangles[face].fromBytes(bytes, MAX_CONFIG_SZ, 
                              triangles, readTriangles);

    DEBUG2_VALUE(" - face=", face);
    DEBUG2_VALUE(" offset=", offset);
    DEBUG2_COMMAND(triangles[face].print();)
  }
  DEBUG_PRINT_END();

  triangles_ptr = &triangles;
  *numTriangles = readTriangles;

  DEBUG2_PRINTLN("Completed reading");

  return offset;
}

int writeTriangleStructure(Triangle *triangles, int numTriangles,
                               int offset) {
  byte bytes[MAX_CONFIG_SZ];

  DEBUG2_VALUE("Writing triangles:", numTriangles);
  DEBUG2_VALUELN(" off:", offset);

  for (byte i = 0; i < MAX_CONFIG_SZ; i++) {
    bytes[i] = 0;
  }

  geometry_config_t *config = (geometry_config_t *)bytes;
  config->version = GEOMETRY_CONFIG_VERSION;
  config->num_objects = numTriangles;
  offset = EEPROM_safe_write(offset, bytes, sizeof (geometry_config_t));

  for (int tri = 0; tri < numTriangles; tri++) {
    int size = triangles[tri].toBytes(bytes, MAX_CONFIG_SZ);
    
    offset = EEPROM_safe_write(offset, bytes, size);
    if (offset < size) {
      DEBUG_ERR("Failed to write squares data");
      break;
    }
    DEBUG2_VALUE("Wrote face=", tri);
    DEBUG2_VALUELN(" offset=", offset);
  }

  DEBUG2_VALUELN("Wrote triangle config. end address=", offset);

  return offset;
}

/*
 *  Perform verifications on a triangle structure
 */
boolean Triangle::verifyTriangleStructure(Triangle *triangles,
                                          int numTriangles, 
                                          geo_led_t numLeds) {

  /* Verify that all edges and vertices have valid values */
  for (geo_id_t t = 0; t < numTriangles; t++) {
    Triangle *tri = &triangles[t];

    for (byte l = 0; l < Triangle::NUM_LEDS; l++) {
      if ((tri->leds[l].pixel != Triangle::NO_LED) && (tri->leds[l].pixel >= numLeds)) {
        DEBUG1_VALUE("Tri:", t);
        DEBUG1_VALUELN(" invalid led:", l);
        return false;
      }
    }

    for (byte e = 0; e < Triangle::NUM_EDGES; e++) {
      if ((tri->edges[e] != NO_EDGE) && (tri->edges[e] >= numTriangles)) {
        DEBUG1_VALUE("Tri:", t);
        DEBUG1_VALUELN(" invalid edge:", e);
        return false;
      }
    }
  }

  /*
   * Verify that all triangles have the correct number of other triangles
   * with edges to them.
   */
  for (geo_id_t verify = 0; verify < numTriangles; verify++) {
    byte edge_count = 0;
    byte vertex_count = 0;

    for (geo_id_t t = 0; t < numTriangles; t++) {
      for (byte e = 0; e < Triangle::NUM_EDGES; e++) {
        if (triangles[t].edges[e] == verify) {
          edge_count++;
        }
      }
      for (byte v = 0; v < Triangle::NUM_VERTICES; v++) {
        for (byte o = 0; o < Triangle::VERTEX_ORDER; o++) {
          if (triangles[t].getVertex(v, o)->id == verify) {
            vertex_count++;
          }
        }
      }
    }

    if (edge_count > Triangle::NUM_EDGES) {
      DEBUG1_VALUE("Tri:", verify);
      DEBUG1_VALUELN(" invalid edge count", edge_count);
      return false;
    } else if (edge_count < Triangle::NUM_EDGES) {
      DEBUG1_VALUE("Tri:", verify);
      DEBUG1_VALUELN(" low edge:", edge_count);
    }

    if (vertex_count > (Triangle::NUM_VERTICES * Triangle::VERTEX_ORDER)) {
      DEBUG1_VALUE("Tri:", verify);
      DEBUG1_VALUELN(" invalid vertex count:", vertex_count);
    } else if (vertex_count > (Triangle::NUM_VERTICES * Triangle::VERTEX_ORDER)) {
      DEBUG1_VALUE("Tri:", verify);
      DEBUG1_VALUELN(" low vertex count:", vertex_count);
    }

  }

  // TODO: Additional validation
  //       - LEDs should be unique
  //       - Traversals (ie right-of, etc) should return to the start

  return true;
}

/* Send updated values to a Pixel chain */
void updateTrianglePixels(Triangle *triangles, int numTriangles,
                          PixelUtil *pixels) {
  boolean update = false;
  int updated = 0;
  for (int tri = 0; tri < numTriangles; tri++) {
    if (triangles[tri].updated) {
      update = true;
      for (byte led = 0; led < Triangle::NUM_LEDS; led++) {
        pixels->setPixelRGB(&(triangles[tri].leds[led]));
      }
      triangles[tri].updated = false;
      updated++;
    }
  }

  if (update) {
    pixels->update();
    DEBUG4_VALUELN("Updated triangles:", updated);
  }
}
