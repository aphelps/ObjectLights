/*
 * Code for communicating with remote modules
 */

#include <Arduino.h>
#include "EEPROM.h"
#include <RS485_non_blocking.h>
#include <SoftwareSerial.h>
#include "SPI.h"
#include "Wire.h"
#include "Adafruit_WS2801.h"


#define DEBUG_LEVEL 0
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


RS485Socket rs485(PIN_RS485_RECV, PIN_RS485_XMIT, PIN_RS485_ENABLE,
		  false); // Whether to enable debugging text in RS485Socket

#define SEND_BUFFER_SIZE (sizeof (rs485_socket_msg_t) + sizeof (msg_hdr_t) + sizeof (msg_max_t) + 64)

byte databuffer[SEND_BUFFER_SIZE];
byte *send_buffer;

void initializeConnect() {
  /* Setup the RS485 connection */  
  rs485.setup();
  send_buffer = rs485.initBuffer(databuffer);
}


#define DEST_ADDR 1 // XXX - Currently hard-coded

void sendInt(int value) {
  send_buffer[0] = (value & 0xFF00) >> 8;
  send_buffer[1] = (value & 0x00FF);
  rs485.sendMsgTo(DEST_ADDR, send_buffer, sizeof (int));
}
