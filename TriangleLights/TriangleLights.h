#ifndef TRIANGLE_LIGHTS
#define TRIANGLE_LIGHTS

#include "TriangleStructure.h"
#include "RS485Utils.h"


extern Triangle *triangles;

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
