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

/* Initialize the message and mode handlers */
void init_modes(Socket **sockets, byte num_sockets);

/* Check for messages and handle program modes */
boolean messages_and_modes(void);

boolean mode_set_all(output_hdr_t *output, void *object,
                     program_tracker_t *tracker);
boolean mode_static_noise(output_hdr_t *output, void *object,
                     program_tracker_t *tracker);

boolean mode_generic_init(msg_program_t *msg,
                          program_tracker_t *tracker,
                          output_hdr_t *output);

void setAllTriangles(Triangle *triangles, int size, uint32_t color);

#endif //OBJECTLIGHTS_TRIANGLELIGHTSMODES_H_H
