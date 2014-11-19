/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Definition of abstract class for ObjectLight geometry
 */

#ifndef GEOMETRY
#define GEOMETRY

class Geometry {
 public:

  /* Constants for no-values */
  static const byte NO_LED = (byte)-1;
  static const byte NO_FACE = (byte)-1;
  static const byte NO_DIRECTION = (byte)-1;
  static const byte NO_INDEX = (byte)-1;
  static const byte NO_EDGE = (byte)-1;
  static const byte NO_VERTEX = (byte)-1;
  static const byte NO_ID = (byte)-1;

  /*
   * Serialization functions
   */

  /* Write this object to a byte buffer, returning the number of bytes used */
  virtual int toBytes(byte *bytes, int size);

  virtual void fromBytes(byte *bytes, int size, Geometry *objects, int numObjects);

  /*
   * Color functions
   */
  virtual void setColor(byte r, byte g, byte b);
  virtual void setColor(uint32_t c);
  virtual void setColor(byte led, byte r, byte g, byte b);
  virtual void setColor(byte led, uint32_t c);

  virtual uint32_t getColor();
  virtual uint32_t getColor(byte led);

  /* 
   * Utility functions 
   */
  virtual void print(byte level);

  /* Return whether this object has LEDs defined */
  boolean hasLeds() { return (leds[0].pixel != NO_LED); }

  byte id;

  RGB leds[];
  byte mark;
  boolean updated; // XXX - Can this be determined some other way to save a byte?

};

#endif
