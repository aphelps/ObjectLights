#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "TriangleLights.h"

volatile uint16_t buttonValue = 0;
void buttonInterrupt(void)
{
  static unsigned long prevTime = 0;
  static int prevValue = LOW;
  long now = millis();
  int value = digitalRead(PUSH_BUTTON_PIN);

  /* Provide a debounce to only change on the first interrupt */
  if ((value == HIGH) && (prevValue == LOW) && (now - prevTime > 500)) {
    buttonValue++;
    prevTime = now;

    DEBUG_VALUE(DEBUG_HIGH, "value=", value);
    DEBUG_VALUE(DEBUG_HIGH, " buttonValue=", buttonValue);
    DEBUG_VALUE(DEBUG_HIGH, " prevTime=", prevTime);
  }

  prevValue = value;
}

int getButtonValue() {
  return buttonValue;
}
