/*
 * Write out a CubeLight configuration
 */
#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>

#include "SPI.h"
#include "Adafruit_WS2801.h"

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

#include "SquareStructure.h"
#include "CubeLights.h"
#include "CubeConfig.h"
#include "CubeConfiguration.h"

boolean wrote_config = false;

#define PIN_DEBUG_LED 13

int numLeds = 45 + FIRST_LED;
PixelUtil pixels;

int numSquares = 6;
Square *squares;

#define MAX_OUTPUTS 4
config_hdr_t config;
output_hdr_t *outputs[MAX_OUTPUTS];

config_value_t val_output, val_output2, val_output3, val_output4;
config_rgb_t rgb_output, rgb_output2;
config_pixels_t pixel_output;
config_mpr121_t mpr121_output;
config_rs485_t rs485_output;

#define MASTER_ADDRESS 0
#define ADDRESS 0

boolean force_write = false; // XXX - Should not be enabled except for debugging

SerialCLI serialcli(128, cliHandler);

void config_init() 
{
  int out = 0;

  rgb_output.hdr.type = HMTL_OUTPUT_RGB;
  rgb_output.hdr.output = out;
  rgb_output.pins[0] = 10; // R
  rgb_output.pins[1] = 11; // G
  rgb_output.pins[2] = 13; // B
  rgb_output.values[0] = 0;
  rgb_output.values[1] = 0;
  rgb_output.values[2] = 0;
  outputs[out] = &rgb_output.hdr;   out++;

  pixel_output.hdr.type = HMTL_OUTPUT_PIXELS;
  pixel_output.hdr.output = out;
  pixel_output.dataPin = 12;
  pixel_output.clockPin = 8;
  pixel_output.numPixels = 45 + FIRST_LED; // XXX - FIRST_LED is still in .h
  pixel_output.type = WS2801_RGB;
  outputs[out] = &pixel_output.hdr; out++;

  mpr121_output.hdr.type = HMTL_OUTPUT_MPR121;
  mpr121_output.hdr.output = out;
  mpr121_output.irqPin = 2;
  mpr121_output.useInterrupt = false;
  for (int i = 0; i < MAX_MPR121_PINS; i++) {
    mpr121_output.thresholds[i] = 0;
  }
  mpr121_output.thresholds[CAP_SENSOR_1] = 
    (CAP_SENSOR_1_TOUCH) | (CAP_SENSOR_1_RELEASE << 4);
  mpr121_output.thresholds[CAP_SENSOR_2] = 
    (CAP_SENSOR_2_TOUCH) | (CAP_SENSOR_2_RELEASE << 4);
  outputs[out] = &mpr121_output.hdr; out++;

  rs485_output.hdr.type = HMTL_OUTPUT_RS485;
  rs485_output.hdr.output = out;
  rs485_output.recvPin = 4; // 2 on board v1, 4 on board v2
  rs485_output.xmitPin = 7;
  rs485_output.enablePin = 5; // 4 on board v1, 5 on board v2
  outputs[out] = &rs485_output.hdr; out++;

  hmtl_default_config(&config);
  config.address = ADDRESS;
  config.num_outputs = out;
  config.flags = 0;

  if (ADDRESS == MASTER_ADDRESS) {
    config.flags |= HMTL_FLAG_MASTER | HMTL_FLAG_SERIAL;
  }

  if (out > MAX_OUTPUTS) {
    DEBUG_ERR("Exceeded maximum outputs");
    DEBUG_ERR_STATE(DEBUG_ERR_INVALID);
  }
}

config_hdr_t readconfig;
config_max_t readoutputs[MAX_OUTPUTS];

int configOffset = -1;

void setup() 
{
  Serial.begin(9600);

  if (force_write) {
    DEBUG_PRINTLN(0, "XXX WARNING: FORCE_WRITE IS ENABLED!!! XXX");
  }

  // Initialize the squares
  squares = buildCube(&numSquares, numLeds, FIRST_LED);
  clearSquares();

  readconfig.address = -1;
  configOffset = hmtl_read_config(&readconfig, 
				  readoutputs, 
				  MAX_OUTPUTS);
  if ((configOffset < 0) ||
      (readconfig.address != config.address) ||
      force_write) {
    // Setup and write the configuration
    config_init();

    configOffset = hmtl_write_config(&config, outputs);
    if (configOffset < 0) {
      DEBUG_ERR("Failed to write config");
    } else {
      wrote_config = true;
    }

  } else {
    DEBUG_VALUELN(DEBUG_LOW, "Read config.  offset=", configOffset);
    memcpy(&config, &readconfig, sizeof (config_hdr_t));
    for (int i = 0; i < config.num_outputs; i++) {

//      uint8_t *buff = (uint8_t *)&readoutputs[i];
//      DEBUG_VALUE(DEBUG_HIGH, " data=", (int)buff);
//      for (uint8_t j = 0; j < sizeof(readoutputs[i]); j++) {
//        DEBUG_HEXVAL(DEBUG_HIGH, " ", buff[j]);
//      }

      if (i >= MAX_OUTPUTS) {
        DEBUG_VALUELN(0, "Too many outputs:", config.num_outputs);
        return;
      }
      outputs[i] = (output_hdr_t *)&readoutputs[i];
    }

    // XXX - This is where we would also read the Cube specific config
    readConfiguration(squares, numSquares, configOffset);
  }

  pinMode(PIN_DEBUG_LED, OUTPUT);

  pixels.init(numLeds, 12, 8); // Should come from config?

  DEBUG_VALUELN(DEBUG_LOW, "Configure initialized.  End address=",
		configOffset);
}

boolean output_data = false;
void loop() 
{
  if (!output_data) {
    DEBUG_VALUE(0, "My address:", config.address);
    DEBUG_VALUELN(0, " Was config written: ", wrote_config);
    hmtl_print_config(&config, outputs);
    output_data = true;
  }

  serialcli.checkSerial();

  //  for (int tri = 0; tri < numSquares; tri++) {
  //    squares[tri].setColor(pixel_color(255, 0, 0));
  //    squares[tri].mark = 0;
  //  }

  blink_value(PIN_DEBUG_LED, config.address, 250, 4);
  delay(10);
}

byte currentPixel = 0;
byte current_face = -1;
byte current_led = -1;

/*
 * CLI Handler to setup the cube geometry
 *
 * l <face> <led> - Toggle the state of an LED on the indicated face
 * s <face> <led> - Set the geometry of the current pixel
 * c - Display current pixel
 * n - Advance to the next pixel
 */
void cliHandler(char **tokens, byte numtokens) {

#if 0
  Serial.print(numtokens);
  Serial.print(" tokens: ");
  for (int token = 0; token < numtokens; token++) {
    if (token != 0) Serial.print(", ");
    Serial.print(tokens[token]);
  }
  Serial.println();
#endif

  switch (tokens[0][0]) {
    case 'l': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);
      setSquareLED(face, led, pixel_color(255, 255, 255));
      DEBUG_VALUE(DEBUG_LOW, "Light Face:", face);
      DEBUG_VALUE(DEBUG_LOW, " LED:", led);
      DEBUG_VALUELN(DEBUG_LOW, " Pixel:", squares[face].leds[led].pixel);
      break;
    }

    case 's': {
      if (numtokens < 3) return;
      byte face = atoi(tokens[1]);
      byte led = atoi(tokens[2]);
      squares[face].setLedPixel(led, currentPixel);
      setSquareLED(face, led, pixel_color(255, 0, 0));
      DEBUG_VALUE(DEBUG_LOW, "Set Face:", face);
      DEBUG_VALUE(DEBUG_LOW, " LED:", led);
      DEBUG_VALUELN(DEBUG_LOW, " Pixel:", currentPixel);
      break;
    }

  case 'c': {
      clearPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "current:", currentPixel);
      break;
    }

    case 'n': {
      clearPixels();
      currentPixel = (currentPixel + 1) % pixels.numPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "current:", currentPixel);
      break;
    }

    case 'p': {
      clearPixels();
      currentPixel = (currentPixel + pixels.numPixels() - 1) % pixels.numPixels();
      pixels.setPixelRGB(currentPixel, 255, 255, 255);
      pixels.update();
      DEBUG_VALUELN(DEBUG_LOW, "next:", currentPixel);
      break;
    }

  case 'w': {
    if (strcmp(tokens[0], "write") == 0) {
      writeConfiguration(squares, numSquares, configOffset);
    }
    break;
  }

  }

}

void clearPixels() {
  // current_face = -1;
  for (byte led = 0; led < pixels.numPixels(); led++) {
    pixels.setPixelRGB(led, 0);
  }
  pixels.update();
  for (int face = 0; face < numSquares; face++) {
    squares[face].setColor(0);
  }
  updateSquarePixels(squares, numSquares, &pixels);
}

void setSquareLED(byte face, byte led, uint32_t color) {
  // Turn off the current led and turn on the new one
  squares[current_face].setColor(current_led, 0);
  squares[face].setColor(led, color);
  current_face = face;
  current_led = led;
  updateSquarePixels(squares, numSquares, &pixels);
}

void clearSquares() {
  for (int face = 0; face < numSquares; face++) {
    for (int led = 0; led < Square::NUM_LEDS; led++) {
      squares[face].setLedPixel(led, -1);
    }
  }
}
