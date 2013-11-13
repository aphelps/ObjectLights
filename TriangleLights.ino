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

#define DEBUG_LED 13

void setup()
{
  Serial.begin(9600);
  DEBUG_PRINTLN(DEBUG_HIGH, "Initializing");

  pinMode(DEBUG_LED, OUTPUT);

  /* Initialize random see by reading from an unconnected analog pin */
  randomSeed(analogRead(3));

  pixels = PixelUtil(numLeds, 12, 11); // HMTL=8,12  Hand=12, 11);
  // pixels = PixelUtil(numLeds, 8, 12); // HMTL=8,12  Hand=12, 11);

  /* Setup the sensors */
  initializePins();

  /* Generate the geometry */
  triangles = buildIcosohedron(&numTriangles, numLeds);
  DEBUG_VALUELN(DEBUG_HIGH, "Inited with numTriangles:", numTriangles);
  //  DEBUG_PRINTLN(DEBUG_HIGH, "Early exit"); return; // XXX

#if 0
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

  // XXX - Color assignements test
  int p = 47; uint32_t color = pixels.pixelColor(64, 0, 0);
  for (int i = 0; i < 3; i++) pixels.setPixelRGB(p+i,color);
  p = p - 3;  color = pixels.pixelColor(0, 64, 0);
  for (int i = 0; i < 3; i++) pixels.setPixelRGB(p+i,color);
  p = p - 3;  color = pixels.pixelColor(0, 0, 64);
  for (int i = 0; i < 3; i++) pixels.setPixelRGB(p+i,color);
  p = p - 3;  color = pixels.pixelColor(64, 64, 0); // yellow
  for (int i = 0; i < 3; i++) pixels.setPixelRGB(p+i,color);
  p = p - 3;  color = pixels.pixelColor(64, 0, 64); // purple
  for (int i = 0; i < 3; i++) pixels.setPixelRGB(p+i,color);
  p = p - 3;  color = pixels.pixelColor(0, 64, 64); // teal
  for (int i = 0; i < 3; i++) pixels.setPixelRGB(p+i,color);

pixels.update();
  while (true);
#endif
}

#define NUM_MODES 10
#define MODE_PERIOD 50
void loop() {
  //setupMode(); return;

  static byte prev_mode = -1;
  byte mode;

  mode = getButtonValue() % NUM_MODES;
  if (mode != prev_mode) {
    DEBUG_MEMORY(DEBUG_HIGH);
  }

  /* Check for update of light sensor value */
  //  sensor_photo();
  switch (mode) {
  case 0: {
#if 0
    pixels.patternOne(MODE_PERIOD);
    pixels.update();
#else
    trianglesSnake(triangles, numTriangles, MODE_PERIOD,
			   prev_mode != mode);

#endif
    break;
  }
  case 1: {
    trianglesTestPattern(triangles, numTriangles, 500,
			 prev_mode != mode);
    break;
  }
  case 2: {
    trianglesCircleCorner(triangles, numTriangles, MODE_PERIOD,
			  prev_mode != mode);
    break;
  }
  case 3: {
    trianglesCircleCorner2(triangles, numTriangles, MODE_PERIOD,
			   prev_mode != mode);
    break;
  }
  case 4: {
    trianglesRandomNeighbor(triangles, numTriangles, MODE_PERIOD,
			    prev_mode != mode);
    break;
  }
  case 5: {
    trianglesLifePattern(triangles, numTriangles, 500,
			 prev_mode != mode);
    break;
  }
  case 6: {
    trianglesLifePattern2(triangles, numTriangles, 500,
			 prev_mode != mode);
    break;
  }
  case 7: {
    trianglesBuildup(triangles, numTriangles, MODE_PERIOD,
		     prev_mode != mode, 0, 0);
    break;
  }
  case 8: {
    trianglesStaticNoise(triangles, numTriangles, MODE_PERIOD,
			 prev_mode != mode);
    break;
  }
  case 9: {
    trianglesSwapPattern(triangles, numTriangles, MODE_PERIOD,
			 prev_mode != mode);
    break;
  }
  }
  updateTrianglePixels(triangles, numTriangles, &pixels);
  prev_mode = mode;

  DEBUG_COMMAND(DEBUG_HIGH,
		static long next_millis = 0;
		if (millis() > next_millis) {
		  if (next_millis % 2 == 0) {
		    digitalWrite(DEBUG_LED, HIGH);
		  } else {
		    digitalWrite(DEBUG_LED, LOW);
		  }
		  next_millis += 251;
		}
		);
}

/*
 * Once the topology of the triangles has been set this can be used
 * to set the individual LEDs within the triangles.
 */
void setupMode() {
  static int prev_value = -1;
  int value;
  static int triangle = 0;

  if (prev_value == -1) {
    triangle = 0;
    prev_value = 0;
  } else {
    value = getButtonValue() % numTriangles;
    if (value != prev_value) {
      triangles[triangle].vertices[0][0]->setColor(0);
      triangles[triangle].vertices[0][1]->setColor(0);
      triangles[triangle].vertices[1][0]->setColor(0);
      triangles[triangle].vertices[1][1]->setColor(0);
      triangles[triangle].vertices[2][0]->setColor(0);
      triangles[triangle].vertices[2][1]->setColor(0);
      triangles[triangle].setColor(0);

      triangle = (triangle + 1) % numTriangles;
      prev_value = value;
      DEBUG_VALUELN(DEBUG_LOW, "tri=", triangle);
    }
  }

  /*
   * Set the current triangles vertex[0] to white
   *
   */
  triangles[triangle].vertices[0][0]->setColor(32, 0, 0);
  triangles[triangle].vertices[0][1]->setColor(32, 0, 0);
  triangles[triangle].setColor(0, 255, 0, 0); // R

  triangles[triangle].vertices[1][0]->setColor(0, 32, 0);
  triangles[triangle].vertices[1][1]->setColor(0, 32, 0);
  triangles[triangle].setColor(1, 0, 255, 0); // G

  triangles[triangle].vertices[2][0]->setColor(0, 0, 32);
  triangles[triangle].vertices[2][1]->setColor(0, 0, 32);
  triangles[triangle].setColor(2, 0, 0, 255); // B
  updateTrianglePixels(triangles, numTriangles, &pixels);
  delay(10);
}
