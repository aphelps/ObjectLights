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

void loop() {

}
