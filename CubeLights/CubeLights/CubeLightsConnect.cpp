/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Code for communicating with remote modules
 ******************************************************************************/

#include <Arduino.h>
#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>
#include "SPI.h"
#include "Wire.h"
#include "FastLED.h"

//#define DEBUG_LEVEL DEBUG_MID
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "HMTLMessaging.h"

#include "PixelUtil.h"
#include "RS485Utils.h"
#include "MPR121.h"

#include "SquareStructure.h"
#include "CubeLights.h"
#include "CubeConfig.h"


RS485Socket rs485;
uint16_t my_address = 0;

#define SEND_BUFFER_SIZE RS485_BUFFER_TOTAL(sizeof (msg_hdr_t) + sizeof (msg_max_t) + 16) // XXX: Could this be smaller?

byte databuffer[SEND_BUFFER_SIZE];
byte *send_buffer; // Pointer to use for start of send data

void initializeConnect() {
  /* Setup the RS485 connection */
  if (!rs485.initialized()) {
    DEBUG_ERR("RS485 was not initialized, check config");
    DEBUG_ERR_STATE(DEBUG_ERR_UNINIT);
  }

  rs485.setup();
  send_buffer = rs485.initBuffer(databuffer);

  DEBUG_VALUE(DEBUG_LOW, "Initialized RS485. address=", my_address);
  DEBUG_VALUELN(DEBUG_LOW, " bufsize=", SEND_BUFFER_SIZE);
}


#define DEST_ADDR 1 // XXX - Currently hard-coded

void sendByte(byte value, byte address) {
  send_buffer[0] = value;
  rs485.sendMsgTo(address, send_buffer, sizeof (int));
}

void sendInt(int value, byte address) {
  send_buffer[0] = (value & 0xFF00) >> 8;
  send_buffer[1] = (value & 0x00FF);
  rs485.sendMsgTo(address, send_buffer, sizeof (int));
  DEBUG_HEXVAL(DEBUG_TRACE, "sendInt: to=0x", address);
  DEBUG_HEXVALLN(DEBUG_TRACE, " val=", value);
}

void sendLong(long value, byte address) {
  send_buffer[0] = (value & 0xFF000000) >> 24;
  send_buffer[1] = (value & 0x00FF0000) >> 16;
  send_buffer[2] = (value & 0x0000FF00) >> 8;
  send_buffer[3] = (value & 0x000000FF);
  rs485.sendMsgTo(address, send_buffer, sizeof (long));
  DEBUG_HEXVAL(DEBUG_TRACE, "sendLong: to=0x", address);
  DEBUG_HEXVALLN(DEBUG_TRACE, " val=", value);
}

void recvData() {
  unsigned int msglen;

  //  const byte *data = rs485.getMsg(my_address, &msglen);
  const byte *data = rs485.getMsg(RS485_ADDR_ANY, &msglen);
  if (data != NULL) {
    DEBUG_VALUELN(DEBUG_HIGH, "recvData: len=", msglen);
  }
}

void sendHMTLValue(uint16_t address, uint8_t output, int value) {
  hmtl_send_value(&rs485, send_buffer, SEND_BUFFER_SIZE,
		  address, output, value);
}

void sendHMTLBlink(uint16_t address, uint8_t output,
		   uint16_t on_period, uint32_t on_color,
		   uint16_t off_period, uint32_t off_color) {
  hmtl_send_blink(&rs485, send_buffer, SEND_BUFFER_SIZE,
		  address, output,
		  on_period, on_color,
		  off_period, off_color);
}

void sendHMTLTimedChange(uint16_t address, uint8_t output,
			 uint32_t change_period,
			 uint32_t start_color,
			 uint32_t stop_color) {
  hmtl_send_timed_change(&rs485, send_buffer, SEND_BUFFER_SIZE,
			 address, output,
			 change_period,
			 start_color,
			 stop_color);
}
