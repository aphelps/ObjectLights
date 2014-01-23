/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 ******************************************************************************/
#ifndef CUBELIGHTS_H
#define CUBELIGHTS_H

#include "RS485Utils.h"
#include "MPR121.h"
#include "SquareStructure.h"

/* Setup all pins */
void initializePins();

/***** Sensor info ********************************************************** */

void handle_sensors();

/***** Range finder *****/
#define PING_TRIG_PIN 9
#define PING_ECHO_PIN 6
#define PING_MAX_CM 100   /* Maximum distance in cm, limits the sensor delay */
#define PING_DELAY_MS 250 /* Minimum time between readings */

extern int range_cm; /* Last value of the range finger */
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
extern MPR121 touch_sensor;

#define CAP_DELAY_MS 250    /* Minimum time between readings */
void sensor_cap_init(void);
void sensor_cap(void);

/***** Cube Light Connectivity ***********************************************/

#define PIN_RS485_RECV     4 // 2 on board v1, 4 on board v2
#define PIN_RS485_XMIT     7
#define PIN_RS485_ENABLE   5 // 4 on board v1, 5 on board v2

extern RS485Socket rs485;

void initializeConnect();
void sendInt(int value);

/***** Cube light modes *******************************************************/

/* Return the current mode value */
int get_current_mode(void);
int get_current_followup(void);

/* Set the mode */
void set_mode(uint8_t new_mode);
void increment_mode(void);
void restore_mode(void);

void set_followup(uint8_t new_followup);
void increment_followup(void);
void restore_followup(void);

typedef struct {
  uint32_t bgColor;
  uint32_t fgColor;
} pattern_args_t;

extern pattern_args_t modeConfig;
extern pattern_args_t followupConfig;

typedef void (*square_mode_t)(Square *squares, int size, int periodms,
				boolean init, pattern_args_t *arg);

extern square_mode_t modeFunctions[];
extern uint16_t modePeriods[];
extern square_mode_t followupFunctions[];
extern uint16_t followupPeriods[];

/* Index into the modeFunctions array */
#define MODE_ALL_ON          0
#define MODE_TEST_PATTERN    1
#define MODE_SETUP_PATTERN   2
#define MODE_RANDOM_NEIGHBOR 3
#define MODE_CYCLE_PATTERN   4
#define MODE_CIRCLE_PATTERN  5
#define MODE_FADE_CYCLE      6
#define MODE_CAP_RESPONSE    7
#define MODE_STATIC_NOISE    8
#define MODE_SWITCH_RANDOM   9
#define MODE_LIGHT_CENTER   10

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
void squaresCapResponse(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg);
void squaresStaticNoise(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg);
void squaresSwitchRandom(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg);
void squaresLightCenter(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg);

#endif
