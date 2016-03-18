//
// Created by Adam Phelps on 3/16/16.
//
#include <FastLED.h>

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_MID
#endif
#include <Debug.h>

#include "Utilities.h"

#include "TriangleLights.h"


void set_all_triangles(Triangle *triangles, int size, CRGB rgb) {
  for (int tri = 0; tri < size; tri++) {
    triangles[tri].setColor(rgb.r, rgb.g, rgb.b);
    triangles[tri].mark = 0;
  }
}


void clear_triangles(Triangle *triangles, int size) {
  set_all_triangles(triangles, size, 0);
}

void randomTriangles(Triangle *triangles, int size) {
  for (int tri = 0; tri < size; tri++) {
    byte red = (byte)random(0, 16);
    byte green = (byte)random(0, 16);
    byte blue = (byte)random(0, 16);
    red = red * red;
    green = green * green;
    blue = blue * blue;

    triangles[tri].setColor(red, green, blue);
  }
}

void wheelTriangles(Triangle *triangles, int size) {
  for (int tri = 0; tri < size; tri++) {
    triangles[tri].setColor(pixel_wheel(map(tri, 0, size - 1, 0, 255)));
  }
}

void binaryTriangles(Triangle *triangles, int size, uint32_t color, int thresh)
{
  for (int tri = 0; tri < size; tri++) {
    boolean set = (random(0, 100) > thresh);
    if (set) triangles[tri].setColor(color);
    else triangles[tri].setColor(0);
  }
}

void randomBinaryTriangles(Triangle *triangles, int size, byte color, int thresh)
{
  for (int tri = 0; tri < size; tri++) {
    boolean red = (random(0, 100) > thresh);
    boolean green = (random(0, 100) > thresh);
    boolean blue = (random(0, 100) > thresh);

    triangles[tri].setColor((red ? color : 0),
                            (blue ? color : 0),
                            (green ? color : 0));
  }
}

void incrementMarkAll(Triangle *triangles, int size, char incr) {
  for (int tri = 0; tri < size; tri++) {
    int value = triangles[tri].mark + incr;
    if (value < 0) triangles[tri].mark = 0;
    else if (value > 255) triangles[tri].mark = 255;
    else triangles[tri].mark = value;
  }
}

/* Adjusted every led by the indicated amount */
void incrementAll(Triangle *triangles, int size,
                  char r, char g, char b) {
  for (int tri = 0; tri < size; tri++) {
    for (byte c = 0; c < 3; c++) {
      int red = triangles[tri].leds[c].red;
      int green = triangles[tri].leds[c].green;
      int blue = triangles[tri].leds[c].blue;

      red = red + r;
      if (red < 0) red = 0;
      else if (red > 255) red = 255;

      green = green + g;
      if (green < 0) green = 0;
      else if (green > 255) green = 255;

      blue = blue + b;
      if (blue < 0) blue = 0;
      else if (blue > 255) blue = 255;

      triangles[tri].setColor(c, (byte)red, (byte)green, (byte)blue);
    }
  }
}