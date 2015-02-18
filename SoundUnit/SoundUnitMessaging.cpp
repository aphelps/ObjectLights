#include <Arduino.h>

#define DEBUG_LEVEL 4
#include "Debug.h"

#include "Socket.h"
#include "RS485Utils.h"

#include "HMTLTypes.h"
#include "HMTLMessaging.h"
#include "HMTLProtocol.h"

#include "SoundUnit.h"

extern RS485Socket rs485;

/*
 * For sending sound data back over RS485.  Set the buffer size large enough
 * to allow for all unleveled column data plus additional sensors
 */

// Size for Sound + Light + Knob
#define MAX_DATA_LEN  \
  (                                                                     \
   sizeof (msg_sensor_data_t) + sizeof (uint16_t) * (NUM_COLUMNS)       \
   + sizeof (msg_sensor_data_t) + sizeof (uint16_t)                     \
   + sizeof (msg_sensor_data_t) + sizeof (uint16_t)                     \
                                                                        )
                          
#define SEND_BUFFER_SIZE (HMTL_MSG_SENSOR_MIN_LEN + MAX_DATA_LEN + 1)
byte rs485_buffer[RS485_BUFFER_TOTAL(SEND_BUFFER_SIZE)];
byte *send_buffer; // Pointer to use for start of send data

void messaging_init() {
  /* Setup the RS485 connection */
  rs485.setup();
  send_buffer = rs485.initBuffer(rs485_buffer);
}

/*
 * Listen for RS485 messages and respond with sound data
 */
boolean messaging_handle() {
  /* Check for message over RS485 */
  unsigned int msglen;
  msg_hdr_t *msg_hdr = hmtl_rs485_getmsg(&rs485, &msglen);
  if (msg_hdr != NULL) {

    uint16_t source_address = RS485_SOURCE_FROM_DATA(msg_hdr);
    DEBUG5_VALUE("Recv type:", msg_hdr->type);
    DEBUG5_VALUE(" len:", msglen);
    DEBUG5_VALUELN(" src:", source_address);
    
    switch (msg_hdr->type) {
      case MSG_TYPE_SENSOR: {
        /*
         * This data length is the unleveled column values plus the two
         * additional sensors.
         */
        uint8_t datalen = MAX_DATA_LEN;

        uint8_t *dataptr;

        uint16_t reply_addr = RS485_ADDR_ANY;

        /* Broadcast sensor data rather than sending just to the source */
        uint16_t len = hmtl_sensor_fmt(send_buffer, SEND_BUFFER_SIZE, 
                                       reply_addr, datalen, &dataptr);
        
        msg_sensor_data_t *sense = (msg_sensor_data_t *)dataptr;
        sense->sensor_type = HMTL_SENSOR_SOUND;
        sense->data_len = NUM_COLUMNS * sizeof (uint16_t);

        uint16_t *sendptr = (uint16_t *)&sense->data;
        
        /* Set the column values */
        for (byte c = 0; c < NUM_COLUMNS; c++) {
          *sendptr = col[c][colCount];
          sendptr++;
        }

        /* Add the light and knob levels */
        sense = (msg_sensor_data_t *)sendptr;
        sense->sensor_type = HMTL_SENSOR_LIGHT;
        sense->data_len = sizeof (uint16_t);
        sendptr = (uint16_t *)&sense->data;
        *sendptr = light_level;
        sendptr++;

        sense = (msg_sensor_data_t *)sendptr;
        sense->sensor_type = HMTL_SENSOR_POT;
        sense->data_len = sizeof (uint16_t);
        sendptr = (uint16_t *)&sense->data;
        *sendptr = knob_level;
        sendptr++;

        DEBUG1_COMMAND(
                       if ((uint16_t)sendptr - (uint16_t)send_buffer != len) {
                         DEBUG1_VALUE("Wrong len:", 
                                      ((uint16_t)sendptr - 
                                       (uint16_t)send_buffer));
                         DEBUG1_VALUELN(" not:", len);
                       }
                       );

        rs485.sendMsgTo(reply_addr, send_buffer, len);
        return true;
        break;
      }
      default: {
        DEBUG1_VALUE("Unhandled msg type:", msg_hdr->type);
        break;
      }
    }
  }

  return false;
}
