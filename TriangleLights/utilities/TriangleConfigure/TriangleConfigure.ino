/*******************************************************************************
 * Author: Adam Phelps
 * License: Creative Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Write out a TriangleLights configuration and provide a CLI for establishing
 * the individual faces, LEDs, and geometry.
 */

#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "FastLED.h"

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "Wire.h"
#include "MPR121.h"
#include "SerialCLI.h"
#include "RS485Utils.h"

#include "ObjectConfiguration.h"
#include "TriangleStructure.h"
#include "TriangleLights.h"

boolean wrote_config = false;

#define PIN_DEBUG_LED 13

#define MAX_OUTPUTS 4
config_hdr_t config;
output_hdr_t *outputs[MAX_OUTPUTS];
config_max_t readoutputs[MAX_OUTPUTS];

config_value_t val_output, val_output2, val_output3, val_output4;
config_rgb_t rgb_output, rgb_output2;
config_pixels_t pixel_output;
config_mpr121_t mpr121_output;
config_rs485_t rs485_output;

SerialCLI serialcli(128, cliHandler);

int configOffset = -1;

// XXX: These should probably come from libraries
RS485Socket rs485;
PixelUtil pixels;

int numTriangles;
Triangle *triangles;

void setup() 
{
  Serial.begin(9600);

  DEBUG2_PRINTLN("*** TriangleConfigure started ***");

  /* Read the current configuration from EEProm */
  configOffset = readHMTLConfiguration(&config,
                                       outputs, readoutputs, MAX_OUTPUTS,
                                       &pixels, &rs485, NULL);


  //  triangles = buildIcosohedron(&numTriangles, pixels.numPixels());
  numTriangles = TRI_ARRAY_SIZE;
  triangles = initTriangles(numTriangles);

  DEBUG1_PRINTLN("Clearing triangle structure");
  for (int t = 0; t < numTriangles; t++) {
    triangles[t].setLedPixels(Geometry::NO_LED, Geometry::NO_LED, 
                              Geometry::NO_LED);
    for (byte e = 0; e < Triangle::NUM_EDGES; e++) {
      triangles[t].setEdge(e, Triangle::NO_ID);
    }
  }

  pinMode(PIN_DEBUG_LED, OUTPUT);

  DEBUG2_VALUELN("*** Configure initialized.  End address=",
                configOffset);
  DEBUG_MEMORY(DEBUG_LOW);
}

boolean output_data = false;
boolean update = false;

void loop() 
{
  if (!output_data) {
    DEBUG1_VALUE("\nMy address:", config.address);
    DEBUG1_VALUELN(" Was config written: ", wrote_config);
    hmtl_print_config(&config, outputs);

    DEBUG1_VALUELN("Triangles: ", numTriangles);
    for (int tri = 0; tri < numTriangles; tri++) {
      triangles[tri].print();
    }

    print_usage();

    output_data = true;
  }

  serialcli.checkSerial();

  if (update) {
    updateTrianglePixels(triangles, numTriangles, &pixels);
  }

  blink_value(PIN_DEBUG_LED, config.address, 250, 4);
  delay(10);
}

byte currentPixel = 0;
byte current_face = -1;
byte current_led = -1;

void print_usage() {
Serial.print(F(" \n"
  "Usage:\n"
  "  c - Light the current pixel\n"
  "  C <led> - Set the current pixel\n"
  "  n - Advance to the next pixel\n"
  "  p - Return to the previous pixel\n"
  " \n"
  "  s <face> <led> - Set the geometry of the current pixel \n"
  "  S <face> <led> <pixel> - Set the geometry to a given pixel\n"
  "  T <face> <led> - Sequence starting from face and led\n"
  "  R <face> - Reverse the 2nd and 3rd leds (common after T)\n"
  "  e <face> <edge> <neighbor> - Set the neighboring face\n"
  " \n"
  "  l <face> <led> - Toggle the state of an LED on the indicated face\n"
  "  f <face> - Light the indicated face\n"
  "  F <face> - Light the indicate face and all neighbors\n"
  "  N - Advance to the next face\n"
  "\n"
  "  P - Print the current configuration\n"
  "  v - Verify the current configuration\n"
  "  read  - Read in the configuration\n"
  "  write - Write out the configuration\n"
  "\n"
  "  h - Print this help text\n"
  "\n"));
}

/*
 *  CLI Handler to setup the geometry         
 */
void cliHandler(char **tokens, byte numtokens) {

  // TODO: Add actions to first set the face/vertex position of each LED, then
  //       assign each edge for all triangles (light the LEDs on that edge)

  switch (tokens[0][0]) {
    case 'h': {
      print_usage();
      break;
    }

    /*
     * Move the current pixel around
     */
    case 'c': {
      clearPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG2_VALUELN("current:", currentPixel);
      break;
    }
    case 'C': {
      if (numtokens < 2) return;
      byte led = atoi(tokens[1]);

      currentPixel = led;
      clearPixels();
      pixels.setPixelRGB(currentPixel, 0, 255, 0);
      pixels.update();
      DEBUG2_VALUELN("set current to:", currentPixel);
      break;
    }
    case 'n': {
      clearPixels();
      currentPixel = (currentPixel + 1) % pixels.numPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG2_VALUELN("current:", currentPixel);
      break;
    }
    case 'p': {
      clearPixels();
      currentPixel = (currentPixel + pixels.numPixels() - 1) % pixels.numPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG2_VALUELN("next:", currentPixel);
      break;
    }

    /*
     * Set the pixel in a face
     */
    case 's': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);

      if (face > numTriangles) return;
      if (led > Triangle::NUM_LEDS) return;

      triangles[face].setLedPixel(led, currentPixel);
      setTriangleLED(face, led, pixel_color(255, 0, 0));
      DEBUG2_VALUE("Set Face:", face);
      DEBUG2_VALUE(" LED:", led);
      DEBUG2_VALUELN(" Pixel:", currentPixel);
      break;
    }

    case 'S': {
      if (numtokens < 4) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);
      uint16_t pixel = atoi(tokens[3]);

      if (face > numTriangles) return;
      if (led > Triangle::NUM_LEDS) return;
      if (pixel > pixels.numPixels()) return;

      triangles[face].setLedPixel(led, pixel);
      setTriangleLED(face, led, pixel_color(255, 0, 0));
      DEBUG2_VALUE("Set Face:", face);
      DEBUG2_VALUE(" LED:", led);
      DEBUG2_VALUELN(" Pixel:", pixel);
      break;
    }


    case 'T': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      uint16_t pixel = atoi(tokens[2]);

      DEBUG2_VALUE("Sequence from face:", face);
      DEBUG2_VALUELN(" pix:", pixel);

      for (int t = face; t < numTriangles; t++) {
        for (int l = 0; l < Triangle::NUM_LEDS; l++) {
          triangles[t].setLedPixel(l, pixel);
          pixel++;
          
          if (pixel >= pixels.numPixels())
            goto T_DONE;
        }
      }
      T_DONE:

      break;
    }
    case 'R': {
      if (numtokens < 2) return;
      byte face = atoi(tokens[1]);

      DEBUG2_VALUELN("Reversing face:", face);

      geo_led_t led1 = triangles[face].getLED(1)->pixel;
      geo_led_t led2 = triangles[face].getLED(2)->pixel;
      triangles[face].setLedPixel(1, led2);
      triangles[face].setLedPixel(2, led1);
      break;
    }
    case 'e': {
      if (numtokens < 4) return;
      byte face = atoi(tokens[1]);
      byte edge = atoi(tokens[2]);
      byte neighbor = atoi(tokens[3]);

      // TODO: Validate
      DEBUG2_VALUE("Set face:", face);
      DEBUG2_VALUE(" edge:", edge);
      DEBUG2_VALUELN(" neighbor:", neighbor);

      triangles[face].setEdge(edge, neighbor);

      break;
    }

    //XXX - Continue here
    
    /*
     * Turn on leds or faces
     */
    case 'l': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);
      DEBUG2_VALUE("Light Face:", face);
      DEBUG2_VALUE(" LED:", led);
      DEBUG2_VALUELN(" Pixel:", triangles[face].leds[led].pixel);
      setTriangleLED(face, led, pixel_color(255, 255, 255));
      break;
    }
    case 'f': 
    case 'F': {
      if (numtokens < 2) return;
      byte face = atoi(tokens[1]);
      if (face > numTriangles) {
        return;
      }

      DEBUG2_PRINT("Light Face:");
      triangles[face].print();
      //      for (int l = 0; l < Triangle::NUM_LEDS; l++) {
      //  DEBUG2_VALUE(" ", triangles[face].getLED(l)->pixel);
      // }
      DEBUG_PRINT_END();
      setTriangleFace(face, pixel_color(255, 255, 255), tokens[0][0] == 'F');
      break;
    }
    case 'N': {
      byte face = (current_face + 1) % numTriangles;

      DEBUG2_PRINT("Light Face:");
      triangles[face].print();

      setTriangleFace(face, pixel_color(255, 255, 255), true);

      break;
    }

    /*
     * Verification
     */
    case 'v': {
      DEBUG1_PRINTLN("*** Verifying triangle structure:");
      if (Triangle::verifyTriangleStructure(triangles, numTriangles, 
                                            pixels.numPixels())) {
        DEBUG1_PRINTLN("\tpassed");
      } else {
        DEBUG1_PRINTLN("\tfailed");
      }
      break;
    }

    case 'P': {
      output_data = false;
      break;
    }

    /*
     * Read and write the configuration
     */
    case 'r': {
      if (strcmp(tokens[0], "read") == 0) {
        DEBUG1_PRINTLN("\n*** READING CONFIGURATION ***\n");

        int configOffset = hmtl_read_config(&config,
                                            readoutputs, 
                                            MAX_OUTPUTS);
        if (configOffset < 0) {
          DEBUG_ERR("Failed to read hmtl config");
          break;
        }

        Triangle *newTriangles;
        int newNumTriangles;
        int newOffset = readTriangleStructure(configOffset, 
                                              &newTriangles,
                                              &newNumTriangles);

        output_data = false;
      }
      break;
    }

    case 'w': {
      if (strcmp(tokens[0], "write") == 0) {
        DEBUG1_PRINTLN("\n*** WRITING CONFIGURATION ***\n");

        int configOffset = hmtl_write_config(&config, outputs);
        if (configOffset < 0) {
          DEBUG_ERR("Failed to write hmtl config");
          break;
        }

        writeTriangleStructure(triangles, numTriangles, configOffset);
      }
      break;
    }
  }
}


/*
 * Clear all pixels and object leds
 */
void clearPixels() {
  for (byte led = 0; led < pixels.numPixels(); led++) {
    pixels.setPixelRGB(led, 0);
  }
  pixels.update();

  for (int face = 0; face < numTriangles; face++) {
    triangles[face].setColor(0);
  }
  updateTrianglePixels(triangles, numTriangles, &pixels);
}

/*
 * Turn off the current led
 */
void setTriangleLED(byte face, byte led, uint32_t color) {
  // Turn off the current led and turn on the new one
  if ((current_face != Geometry::NO_FACE) && (current_led != Geometry::NO_LED)) {
    triangles[current_face].setColor(current_led, 0);
  }

  triangles[face].setColor(led, color);

  current_face = face;
  current_led = led;

  update = true;
}

/*
 * Turn off the current face and turn on the indicated one
 */
void setTriangleFace(byte face, uint32_t color, boolean neighbors) {
  for (int f = 0; f < numTriangles; f++) {
    triangles[f].setColor(0);
  }

  triangles[face].setColor(color);
  triangles[face].setColor(0, 128,   0,   0);
  triangles[face].setColor(1, 0,   128,   0);
  triangles[face].setColor(2, 0,     0, 128);

  current_face = face;

  if (neighbors) {
    if (triangles[face].getEdge(0) != NULL)
      triangles[face].getEdge(0)->setColor(128, 0, 0);

    if (triangles[face].getEdge(1) != NULL)
      triangles[face].getEdge(1)->setColor(0, 128, 0);

    if (triangles[face].getEdge(2) != NULL)
      triangles[face].getEdge(2)->setColor(0, 0, 128);

#define TEST
#ifdef TEST
    Triangle *tri;

    byte index;

    for (index = 0; (index == 0) || (tri != NULL); index++) {
      tri = triangles[face].getVertex(0, index);
      if (tri != NULL) tri->setColor(4, 0, 0);
    }

    for (index = 0; (index == 0) || (tri != NULL); index++) {
      tri = triangles[face].getVertex(1, index);
      if (tri != NULL) tri->setColor(0, 4, 0);
    }

    for (index = 0; (index == 0) || (tri != NULL); index++) {
      tri = triangles[face].getVertex(2, index);
      if (tri != NULL) tri->setColor(0, 0, 4);
    }


#else
    for (byte o = 0; o < Triangle::VERTEX_ORDER; o++) {
      tri = triangles[face].getVertex(0, o);
      if (tri != NULL)
        tri->setColor(4, 0, 0);

      tri = triangles[face].getVertex(1, o);
      if (tri != NULL)
        tri->setColor(0, 4, 0);

      tri = triangles[face].getVertex(2, o);
      if (tri != NULL)
        tri->setColor(0, 0, 4);
    }
#endif
  }

  update = true;
}
