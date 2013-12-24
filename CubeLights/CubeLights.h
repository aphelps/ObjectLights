/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/
#ifndef CUBELIGHTS_H
#define CUBELIGHTS_H

#include "SquareStructure.h"

/* Setup all pins */
void initializePins();

// XXX - Below here is old

/***** LED modes **************************************************************/

/* Return the current mode value */
int get_current_mode(void);

/* Set the mode */
void set_current_mode(uint8_t new_mode);
void restore_current_mode(void);

/* Mode for lights */
#define MODE_SET_ALL          0
#define MODE_SWAP_ONE         1
#define MODE_FADE_ONE         2
#define MODE_COUNT_UP         3
#define MODE_FLASH_ORDERED    4
#define MODE_RANDOM_FADES     5
#define MODE_SENSE_DISTANCE   6

#define MODE_TOTAL            7 /* 1+ higest mode value */

/* Definition of sign mode function */
typedef int (*mode_function_t)(void *arg);

/* Mode functions */
int mode_set_all(void *arg);
int mode_swap_one(void *arg);
int mode_fade_one(void *arg);
int mode_count_up(void *arg);
int mode_flash_ordered(void *arg);
int mode_random_fades(void *arg);
int mode_sense_distance(void *arg);

void send_update();

/***** Sensor info ********************************************************** */

/***** Range finder *****/
#define PING_TRIG_PIN 9
#define PING_ECHO_PIN 6
#define PING_MAX_CM 100   /* Maximum distance in cm, limits the sensor delay */
#define PING_DELAY_MS 250 /* Minimum time between readings */

extern uint16_t range_cm; /* Last value of the range finger */
void sensor_range(void);  /* Update the range finder value */

/***** Photo sensor *****/
#define PHOTO_PIN A0
#define PHOTO_THRESHOLD_LOW  85  /* Consider light off when below this level */
#define PHOTO_THRESHOLD_HIGH 100 /* Consider light on when above this level */
#define PHOTO_DELAY_MS 100       /* Minimum time between readings */

extern uint16_t photo_value; /* Last value of the photo sensor */
extern boolean photo_dark;   /* If its "dark" based on threshold values */
void sensor_photo(void);     /* Update the photo sensor values */

/***** Capacitive side sensors *****/
#define CAP_TOUCH_MAX 12
extern boolean touch_states[];

#define NUM_CAP_SENSORS 2
#define CAP_TOUCH_1 0
#define CAP_TOUCH_2 2

#define CAP_TOUCH_PIN 2

#define CAP_DELAY_MS 250    /* Minimum time between readings */
void sensor_cap_init(void);
void sensor_cap(void);


/*
 * Cube light mores
 */

typedef struct {
  uint32_t bgColor;
  uint32_t fgColor;
} pattern_args_t;

typedef void (*square_mode_t)(Square *squares, int size, int periodms,
				boolean init, pattern_args_t *arg);

void squaresTestPattern(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void squaresSetupPattern(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void squaresRandomNeighbor(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void squaresCyclePattern(Square *squares, int size, int periodms,
			 boolean init, pattern_args_t *arg);
void squaresCirclePattern(Square *squares, int size, int periodms,
			 boolean init, pattern_args_t *arg);
void squaresFadeCycle(Square *squares, int size, int periodms,
		      boolean init, pattern_args_t *arg);
void squaresAllOn(Square *squares, int size, int periodms,
		  boolean init, pattern_args_t *arg);

#endif
