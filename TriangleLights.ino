#include "SPI.h"
#include "Adafruit_WS2801.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "PixelUtil.h"

#include "TriangleStructure.h"
#include "TriangleLights.h"

#define LIGHT_SENSOR A0

PixelUtil pixels(50, 12, 11);

void setup()
{
  Serial.begin(9600);

  randomSeed(analogRead(3));

  /* Configure the mode toggle switch */
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(0, buttonInterrupt, CHANGE);

  /* Turn on input pullup on analog light sensor pin */
  digitalWrite(LIGHT_SENSOR, HIGH);
}

#define NUM_MODES 2
#define MODE_PERIOD 50
void loop() {
  int mode = getButtonValue() % NUM_MODES;

  switch (mode) {
  case 0: pixels.patternRed(MODE_PERIOD); break;
  case 1: pixels.patternGreen(MODE_PERIOD); break;
  case 2: pixels.patternBlue(MODE_PERIOD); break;
  case 3: pixels.patternOne(MODE_PERIOD); break;
  }

  delay(10);
}
