//
// Created by Adam Phelps on 3/16/16.
//

#ifndef OBJECTLIGHTS_UTILITIES_H
#define OBJECTLIGHTS_UTILITIES_H

#include "TriangleLights.h"

void set_all_triangles(Triangle *triangles, int size, CRGB rgb);
void clear_triangles(Triangle *triangles, int size);
void randomTriangles(Triangle *triangles, int size);
void wheelTriangles(Triangle *triangles, int size);
void binaryTriangles(Triangle *triangles, int size, uint32_t color, int thresh);
void randomBinaryTriangles(Triangle *triangles, int size, byte color, int thresh);
void incrementMarkAll(Triangle *triangles, int size, char incr);
void incrementAll(Triangle *triangles, int size, char r, char g, char b);

#endif //OBJECTLIGHTS_UTILITIES_H
