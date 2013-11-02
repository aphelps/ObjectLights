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


void trianglesTestPattern(Triangle *triangles, int size, int periodms,
			  boolean init);
void trianglesRandomNeighbor(Triangle *triangles, int size, int periodms,
			     boolean init);
void trianglesSwapPattern(Triangle *triangles, int size, int periodms,
			  boolean init);
void trianglesLifePattern(Triangle *triangles, int size, int periodms,
			  boolean init);
void trianglesLifePattern2(Triangle *triangles, int size, int periodms,
			  boolean init);
void trianglesCircleCorner(Triangle *triangles, int size, int periodms,
			   boolean init);
void trianglesBuildup(Triangle *triangles, int size, int periodms,
		      boolean init, uint32_t fgColor, uint32_t bgColor);
void trianglesStaticNoise(Triangle *triangles, int size, int periodms,
			  boolean init);
void trianglesCircleCorner2(Triangle *triangles, int size, int periodms,
			    boolean init);

#endif
