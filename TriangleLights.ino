#include "SPI.h"
#include "Adafruit_WS2801.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "PixelUtil.h"

#include "TriangleStructure.h"
#include "TriangleLights.h"

int numLeds = 50;
PixelUtil pixels(numLeds, 12, 11);

int numTriangles = 0;
Triangle **triangles;


void setup()
{
  Serial.begin(9600);

  randomSeed(analogRead(3));

  /* Setup the sensors */
  initializePins();

  /* Generate the geometry */
  triangles = buildIcosohedron();
  numTriangles = 20;

  /* Set the pixel values for the triangles */
  int led = numLeds - 1;
  for (int i = 0; i < numTriangles; i++) {
    DEBUG_VALUELN(DEBUG_HIGH, "led:", led);
    // XXX - There is no intelligence here.  This is done from highest down
    // so that when wiring the end led should be placed first.
    if (led >= 2) {
      triangles[i]->setLedPixels(led, led - 1, led - 2);
      led -= 3;
    }
  }
}

#define NUM_MODES 2
#define MODE_PERIOD 50
void loop() {
  /* Check for update of light sensor value */
  sensor_photo();

  int mode = getButtonValue() % NUM_MODES;

  switch (mode) {
  case 0: trianglesTestPattern(triangles, numTriangles, 500);
  case 1: trianglesTestPattern(triangles, numTriangles, 500);
  }
  pixels.update();

  delay(10);
}
