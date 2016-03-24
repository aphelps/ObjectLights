/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 *
 * Headers for TriangleLights
 ******************************************************************************/

#ifndef TRIANGLE_LIGHTS
#define TRIANGLE_LIGHTS

#include "TriangleStructure.h"
#include "RS485Utils.h"


extern Triangle *triangles;

/***** Sensors ****************************************************************/

/* Push button */
#define PUSH_BUTTON_INTERRUPT 1
#define PUSH_BUTTON_PIN       3

byte get_button_value();

/* Photo sensor */
#define PHOTO_PIN A0
#define PHOTO_THRESHOLD_LOW  250  /* Consider light off when below this level */
#define PHOTO_THRESHOLD_HIGH 300 /* Consider light on when above this level */

uint16_t get_photo_value();

/* Potentiometer */
#define POT_PIN A4
uint16_t get_pot_value();

// Setup all sensor pins
void initializePins();

// Check all sensors
#define SENSOR_DELAY_MS 10
void update_sensors();

/***** Triangle Light Connectivity ********************************************/

extern RS485Socket rs485;
extern byte my_address;


/*
 * Triangle light modes
 */

typedef struct {
  uint32_t bgColor;
  uint32_t fgColor;
} pattern_args_t;

typedef void (*triangle_mode_t)(Triangle *triangles, int size, int periodms,
				boolean init, pattern_args_t *arg);

void trianglesTestPattern(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void trianglesRandomNeighbor(Triangle *triangles, int size, int periodms,
			     boolean init, pattern_args_t *arg);
void trianglesSwapPattern(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void trianglesLifePattern(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void trianglesLifePattern2(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void trianglesCircleCorner(Triangle *triangles, int size, int periodms,
			   boolean init, pattern_args_t *arg);
void trianglesBuildup(Triangle *triangles, int size, int periodms,
		      boolean init, pattern_args_t *arg);
void trianglesStaticNoise(Triangle *triangles, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void trianglesCircleCorner2(Triangle *triangles, int size, int periodms,
			    boolean init, pattern_args_t *arg);
void trianglesCircle(Triangle *triangles, int size, int periodms,
			   boolean init, pattern_args_t *arg);
void trianglesSnake(Triangle *triangles, int size, int periodms,
		    boolean init, pattern_args_t *arg);
void trianglesSnake2(Triangle *triangles, int size, int periodms,
		    boolean init, pattern_args_t *arg);
void trianglesSetAll(Triangle *triangles, int size, int periodms,
		     boolean init, pattern_args_t *arg);
void trianglesLooping(Triangle *triangles, int size, int periodms,
		      boolean init, pattern_args_t *arg);
void trianglesVertexShift(Triangle *triangles, int size, int periodms,
		      boolean init, pattern_args_t *arg);
void trianglesVertexMerge(Triangle *triangles, int size, int periodms,
		      boolean init, pattern_args_t *arg);
void trianglesVertexMergeFade(Triangle *triangles, int size, int periodms,
		      boolean init, pattern_args_t *arg);


/* Serial input handling */
#define MAX_CLI_LEN 32
void cliRead();

#endif
