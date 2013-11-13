#ifndef TRIANGLE_LIGHTS
#define TRIANGLE_LIGHTS

#include "TriangleStructure.h"

#define PUSH_BUTTON_PIN 2
void buttonInterrupt(void);
int getButtonValue();

/* Photo sensor */
#define PHOTO_PIN A0
#define PHOTO_THRESHOLD_LOW  85  /* Consider light off when below this level */
#define PHOTO_THRESHOLD_HIGH 100 /* Consider light on when above this level */
#define PHOTO_DELAY_MS 100       /* Minimum time between readings */

extern uint16_t photo_value; /* Last value of the photo sensor */
extern boolean photo_dark;   /* If its "dark" based on threshold values */
void sensor_photo(void);     /* Update the photo sensor values */


#define LIGHT_SENSOR_PIN A0
int getLightValue();

void initializePins();

/* Triangle light modes */
typedef void (*triangle_mode_t)(Triangle *triangles, int size, int periodms,
				boolean init, void *arg);

void trianglesTestPattern(Triangle *triangles, int size, int periodms,
			  boolean init, void *arg);
void trianglesRandomNeighbor(Triangle *triangles, int size, int periodms,
			     boolean init, void *arg);
void trianglesSwapPattern(Triangle *triangles, int size, int periodms,
			  boolean init, void *arg);
void trianglesLifePattern(Triangle *triangles, int size, int periodms,
			  boolean init, void *arg);
void trianglesLifePattern2(Triangle *triangles, int size, int periodms,
			  boolean init, void *arg);
void trianglesCircleCorner(Triangle *triangles, int size, int periodms,
			   boolean init, void *arg);
void trianglesBuildup(Triangle *triangles, int size, int periodms,
		      boolean init, void *arg);
void trianglesStaticNoise(Triangle *triangles, int size, int periodms,
			  boolean init, void *arg);
void trianglesCircleCorner2(Triangle *triangles, int size, int periodms,
			    boolean init, void *arg);
void trianglesSnake(Triangle *triangles, int size, int periodms,
		    boolean init, void *arg);

/* Serial input handling */
#define MAX_CLI_LEN 32
void cliRead();

#endif
