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
#include "Adafruit_WS2801.h"


#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "GeneralUtils.h"
#include "EEPromUtils.h"
#include "HMTLTypes.h"
#include "PixelUtil.h"
#include "RS485Utils.h"
#include "MPR121.h"

#include "SquareStructure.h"
#include "CubeLights.h"
#include "CubeConfig.h"


RS485Socket rs485;
byte my_address = 0;

#define SEND_BUFFER_SIZE (sizeof (rs485_socket_msg_t) + sizeof (msg_hdr_t) + sizeof (msg_max_t) + 16) // XXX: Could this be smaller?

byte databuffer[SEND_BUFFER_SIZE];
byte *send_buffer; // Pointer to use for start of send data

void initializeConnect() {
  /* Setup the RS485 connection */
  if (!rs485.initialized) {
    DEBUG_ERR("RS485 was not initialized, check config");
    DEBUG_ERR_STATE(DEBUG_ERR_UNINIT);
  }

  rs485.setup();
  send_buffer = rs485.initBuffer(databuffer);

  DEBUG_VALUE(DEBUG_LOW, "Initialized RS485. address=", my_address);
  DEBUG_VALUELN(DEBUG_LOW, " bufsize=", SEND_BUFFER_SIZE);
}


#define DEST_ADDR 1 // XXX - Currently hard-coded

void sendInt(int value) {
  send_buffer[0] = (value & 0xFF00) >> 8;
  send_buffer[1] = (value & 0x00FF);
  rs485.sendMsgTo(DEST_ADDR, send_buffer, sizeof (int));
}

void recvData() {
  unsigned int msglen;

  //  const byte *data = rs485.getMsg(my_address, &msglen);
  const byte *data = rs485.getMsg(RS485_ADDR_ANY, &msglen);
  if (data != NULL) {
    DEBUG_VALUELN(DEBUG_HIGH, "recvData: len=", msglen);
  }
}
