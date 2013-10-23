#include "SPI.h"
#include "Adafruit_WS2801.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "PixelUtil.h"

#include "TriangleStructure.h"
#include "TriangleLights.h"

int numLeds = 50;
PixelUtil pixels;

int numTriangles = 0;
Triangle *triangles;

void setup()
{
  Serial.begin(9600);
  DEBUG_PRINTLN(DEBUG_HIGH, "Initializing");

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(3));

  pixels = PixelUtil(numLeds, 8, 12); // 12, 11);

  /* Setup the sensors */
  initializePins();

  /* Generate the geometry */
  triangles = buildIcosohedron(&numTriangles);
  DEBUG_VALUELN(DEBUG_HIGH, "Inited with numTriangles:", numTriangles);
  //  DEBUG_PRINTLN(DEBUG_HIGH, "Early exit"); return; // XXX

  /* Set the pixel values for the triangles */
  int led = numLeds - 1;
  for (int i = 0; i < numTriangles; i++) {
    // XXX - There is no intelligence here.  This is done from highest down
    // so that when wiring the end led should be placed first.
    if (led >= 2) {
      DEBUG_VALUE(DEBUG_HIGH, "Setting leds for tri:", i);
      DEBUG_VALUE(DEBUG_HIGH, " ", led);
      DEBUG_VALUE(DEBUG_HIGH, " ", led - 1);
      DEBUG_VALUELN(DEBUG_HIGH, " ", led - 2);

      triangles[i].setLedPixels(led, led - 1, led - 2);
      led -= 3;
    } else {
      DEBUG_VALUELN(DEBUG_HIGH, "No leds for tri:", i);
    }
  }
}

#define NUM_MODES 3
#define MODE_PERIOD 50
void loop() {
  static byte prev_mode = -1;
  byte mode;

  mode = 1; //getButtonValue() % NUM_MODES;
  prev_mode = mode;

  /* Check for update of light sensor value */
  //  sensor_photo();
  switch (mode) {
  case 0: {
#if 1
    pixels.patternRed(MODE_PERIOD);
#endif
#if 1
    pixels.update();
#endif
    break;
  }
  case 1: {
    trianglesTestPattern(triangles, numTriangles, 500,
			 prev_mode != mode);
    break;
  }
  case 2: {
    trianglesRandomNeighbor(triangles, numTriangles, 250,
			    prev_mode != mode);
    break;
  }
  }
  updateTrianglePixels(triangles, numTriangles, &pixels);

  delay(10);
}
