/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2016
 ******************************************************************************/

#ifndef OBJECTLIGHTS_TRIANGLELIGHTSMODES_H_H
#define OBJECTLIGHTS_TRIANGLELIGHTSMODES_H_H

#include "Arduino.h"
#include "Socket.h"

#include <ProgramManager.h>
#include <HMTLMessaging.h>
#include <HMTLPrograms.h>
#include <HMTLProtocol.h>


/* Wireless pendant programs */
#define TRIANGLES_SET_ALL      0x20
#define TRIANGLES_STATIC_NOISE 0x21
#define TRIANGLES_SNAKES_2     0x22

typedef struct {
  uint16_t period_ms; // 2B

  uint8_t data[0];
} mode_hdr_t;

typedef struct {
  mode_hdr_t hdr;     // 2B

  CRGB bgColor;       // 3B
  CRGB fgColor;       // 3B
  uint8_t  data[4];   // 4B

  // Total: 10B

  unsigned long last_change_ms;
} mode_data_t;

#define SNAKE2_LENGTH 12
typedef struct {
  mode_hdr_t hdr; // 2B

  CRGB bgColor;   // 3B
  byte colorMode; // 1B

  byte snakeTriangles[SNAKE2_LENGTH];
  byte snakeVertices[SNAKE2_LENGTH]; // TODO: These values are 0-2, reduce size!
  byte currentIndex;

  unsigned long last_change_ms;
} mode_snake_data_t;


/* Initialize the message and mode handlers */
void init_modes(Socket **sockets, byte num_sockets);

/* Check for messages and handle program modes */
boolean messages_and_modes(void);

/* Check the toggle button and change mode if pressed */
void update_mode_from_button();

/* Set the current mode */
boolean set_mode(byte mode, boolean broadcast);

/* Issue initial commands */
void startup_commands();


/*
 * Programs
 */
boolean mode_set_all(output_hdr_t *output, void *object,
                     program_tracker_t *tracker);
boolean mode_static_noise(output_hdr_t *output, void *object,
                          program_tracker_t *tracker);
boolean mode_snakes_2(output_hdr_t *output, void *object,
                          program_tracker_t *tracker);

boolean mode_generic_init(msg_program_t *msg,
                          program_tracker_t *tracker,
                          output_hdr_t *output);
boolean mode_snakes_init(msg_program_t *msg,
                          program_tracker_t *tracker,
                          output_hdr_t *output);

void setAllTriangles(Triangle *triangles, int size, uint32_t color);

#endif //OBJECTLIGHTS_TRIANGLELIGHTSMODES_H_H
