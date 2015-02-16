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

//#define DEBUG_LEVEL DEBUG_TRACE
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

/*
 * There appears to be some time needed between sending a message and checking
 * for received data.
 */
#define MIN_SEND_TO_READ_MS 10
unsigned long last_sent_time = 0;

void initializeConnect() {
  /* Setup the RS485 connection */
  if (!rs485.initialized()) {
    DEBUG_ERR("RS485 was not initialized, check config");
    DEBUG_ERR_STATE(DEBUG_ERR_UNINIT);
  }

  rs485.setup();
  send_buffer = rs485.initBuffer(databuffer);

  DEBUG2_VALUE("Initialized RS485. address=", my_address);
  DEBUG2_VALUELN(" bufsize=", SEND_BUFFER_SIZE);
}


void sendByte(byte value, byte address) {
  send_buffer[0] = value;
  rs485.sendMsgTo(address, send_buffer, sizeof (int));
  DEBUG5_HEXVAL("sendInt: to=0x", address);
  DEBUG5_HEXVALLN(" val=", value);
  last_sent_time = millis();
}

void sendInt(int value, byte address) {
  send_buffer[0] = (value & 0xFF00) >> 8;
  send_buffer[1] = (value & 0x00FF);
  rs485.sendMsgTo(address, send_buffer, sizeof (int));
  DEBUG5_HEXVAL("sendInt: to=0x", address);
  DEBUG5_HEXVALLN(" val=", value);
  last_sent_time = millis();
}

void sendLong(long value, byte address) {
  send_buffer[0] = (value & 0xFF000000) >> 24;
  send_buffer[1] = (value & 0x00FF0000) >> 16;
  send_buffer[2] = (value & 0x0000FF00) >> 8;
  send_buffer[3] = (value & 0x000000FF);
  rs485.sendMsgTo(address, send_buffer, sizeof (long));
  DEBUG5_HEXVAL("sendLong: to=0x", address);
  DEBUG5_HEXVALLN(" val=", value);
  last_sent_time = millis();
}

void recvData() {
  unsigned int msglen;

  //  const byte *data = rs485.getMsg(my_address, &msglen);
  const byte *data = rs485.getMsg(RS485_ADDR_ANY, &msglen);
  if (data != NULL) {
    DEBUG5_VALUELN("recvData: len=", msglen);
  }
}

void sendHMTLValue(uint16_t address, uint8_t output, int value) {
  DEBUG5_PRINTLN("sendHMTLValue");
  hmtl_send_value(&rs485, send_buffer, SEND_BUFFER_SIZE,
		  address, output, value);
  last_sent_time = millis();
}

void sendHMTLBlink(uint16_t address, uint8_t output,
		   uint16_t on_period, uint32_t on_color,
		   uint16_t off_period, uint32_t off_color) {
  DEBUG5_PRINTLN("sendHMTLBlink");
  hmtl_send_blink(&rs485, send_buffer, SEND_BUFFER_SIZE,
		  address, output,
		  on_period, on_color,
		  off_period, off_color);
  last_sent_time = millis();
}

void sendHMTLTimedChange(uint16_t address, uint8_t output,
			 uint32_t change_period,
			 uint32_t start_color,
			 uint32_t stop_color) {
  DEBUG5_PRINTLN("sendHMTLTimedChange");
  hmtl_send_timed_change(&rs485, send_buffer, SEND_BUFFER_SIZE,
			 address, output,
			 change_period,
			 start_color,
			 stop_color);
  last_sent_time = millis();
}

void sendHMTLSensorRequest(uint16_t address) {
  DEBUG5_PRINTLN("sendHMTLSensorRequest");
  hmtl_send_sensor_request(&rs485, send_buffer, SEND_BUFFER_SIZE, address);
  last_sent_time = millis();
}


unsigned long sensor_check_time = 0;
msg_hdr_t *sensor_msg = NULL;

/*
 * RS485 Message handling
 */
void handle_messages() {
  unsigned long now = millis();
  sensor_msg = NULL;

  if (now - last_sent_time > MIN_SEND_TO_READ_MS) {
    /* Check for messages */
    unsigned int msglen;
    msg_hdr_t *msg_hdr = hmtl_rs485_getmsg(&rs485, &msglen, my_address);
    if (msg_hdr) {
      DEBUG4_VALUE("Recv type:", msg_hdr->type);
      if (msg_hdr->type == MSG_TYPE_SENSOR) {
        sensor_msg = msg_hdr;
      }
    }
    DEBUG_PRINT_END();
  }

  /* Send sensor check */
  if (now >= sensor_check_time) {
    sendHMTLSensorRequest(ADDRESS_SOUND_UNIT);
    sensor_check_time = now + 1000;
    last_sent_time = now;
  }
}
