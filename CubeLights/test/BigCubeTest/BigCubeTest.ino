/*
 * Written by Adam Phelps, amp@cs.stanford.edu, 2013-2014
 * 
 * This is a trivial example using multiple MPR121 sensors
 */

//#define DEBUG_LEVEL DEBUG_HIGH
#include <Debug.h>

#include <Arduino.h>
#include "MPR121.h"
#include <Wire.h>

//#define ADAFRUIT_LIQUID
#if ADAFRUIT_LIQUID
  #include "LiquidCrystal.h"
  LiquidCrystal lcd(0);
#else
  // This claims to be much faster: http://forums.adafruit.com/viewtopic.php?f=19&t=21586&p=113177
  #include <LiquidTWI.h>
  LiquidTWI lcd(0);
#endif


#define DEBUG_LED 13


#define NUM_SENSORS 2

byte irq_pins[NUM_SENSORS] = {
  3,
  5
};

byte addresses[NUM_SENSORS] = {
  0x5A, // ADD=GND (default)
  0x5B, // ADD=VCC
  //  0x5C, // ADD=SDA
  //  0x5D, // ADD=SCL
};

MPR121 sensors[NUM_SENSORS];

void setup() {
  Serial.begin(9600);
  Serial.println("MPR121 Multiple initializing");

  Wire.begin();

  for (byte sensor = 0; sensor < NUM_SENSORS; sensor++) {
    sensors[sensor] = MPR121(
                             irq_pins[sensor],  // triggered/interupt pin
                             false,             // interrupt mode?
                             addresses[sensor], // START_ADDRESS = 0x5A
                             true,              // use touch times
                             false              // use auto config and reconfig
                             );
    /*
     * Set default touch and release thresholds
     */
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      sensors[sensor].setThreshold(i, 15, 2);
    }  
  }

  pinMode(DEBUG_LED, OUTPUT);

  Serial.println("MPR121 sensors initialized");

  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
  lcd.setCursor(0, 1);
}

boolean debug_on = false;
boolean data_changed = false;

void update_lcd() {
  uint32_t now = millis();
  static uint32_t last_update = 0;

  if (data_changed) {
    lcd.setCursor(0, 0);
    lcd.print("0:");
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      lcd.print(sensors[0].touched(i));
    }

    lcd.setCursor(0, 1);
    lcd.print("1:");
    for (byte i = 0; i < MPR121::MAX_SENSORS; i++) {
      lcd.print(sensors[1].touched(i));
    }

    data_changed = false;
  }
}


void loop() {
  boolean changed = false;

  for (byte sensor = 0; sensor < NUM_SENSORS; sensor++) {

    if (sensors[sensor].readTouchInputs()) {
      changed = true;

      for (int i = 0; i < MPR121::MAX_SENSORS; i++) {
        if (sensors[sensor].changed(i)) {

          data_changed = true;

          Serial.print(sensor);
          Serial.print(" - pin ");
          Serial.print(i);
          Serial.print(":");
	
          if (sensors[sensor].touched(i)) {
            Serial.println(" Sensed");
          } else {
            Serial.print(" Released after ");
            Serial.print(sensors[sensor].touchTime(i));
            Serial.println(" ms ");
          }
        }
      }
    }
  }

  if (changed) {
    // Flash the on-board LED when there is a change of state
    debug_on = !debug_on;
    if (debug_on) digitalWrite(DEBUG_LED, HIGH);
    else digitalWrite(DEBUG_LED, HIGH);
  }

  //checkSerial();

  update_lcd();
}
