/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/
#ifndef CUBELIGHTS_H
#define CUBELIGHTS_H

#include "RS485Utils.h"
#include "MPR121.h"
#include "SquareStructure.h"

/* Setup all pins */
void initializePins();

/***** Geometry and LEDS **************************************************** */
extern PixelUtil pixels;
extern Square *squares;

/***** Sensor info ********************************************************** */

/* All sensor info is recorded in a bit mask */
extern uint32_t sensor_state;

#define SENSE_TOUCH_1     0x00000001
#define SENSE_CHANGE_1    0x00000002
#define SENSE_DOUBLE_1    0x00000004
#define SENSE_LONG_1      0x00000008 // XXX - Need to implement

#define SENSE_TOUCH_2     0x00000010
#define SENSE_CHANGE_2    0x00000020
#define SENSE_DOUBLE_2    0x00000040
#define SENSE_LONG_2      0x00000080 // XXX - Need to implement

#define SENSE_DOUBLE_BOTH 0x01000000 // XXX - Can these be just combos?
#define SENSE_LONG_BOTH   0x02000000 // XXX - Can these be just combos?

#define SENSE_RANGE_SHORT 0x10000000
#define SENSE_RANGE_MID   0x20000000
#define SENSE_RANGE_LONG  0x40000000

#define SENSE_TOUCH_ALL   0x00000011
#define SENSE_CHANGE_ALL  0x00000022

#define CHECK_TOUCH_1()     (sensor_state & SENSE_TOUCH_1)
#define CHECK_CHANGE_1()    (sensor_state & SENSE_CHANGE_1)
#define CHECK_DOUBLE_1()    (sensor_state & SENSE_DOUBLE_1)
#define CHECK_LONG_1()      (sensor_state & SENSE_LONG_1)
#define CHECK_TAP_1()       (CHECK_TOUCH_1() && CHECK_CHANGE_1())

#define CHECK_TOUCH_2()     (sensor_state & SENSE_TOUCH_2)
#define CHECK_CHANGE_2()    (sensor_state & SENSE_CHANGE_2)
#define CHECK_DOUBLE_2()    (sensor_state & SENSE_DOUBLE_2)
#define CHECK_LONG_2()      (sensor_state & SENSE_LONG_2)
#define CHECK_TAP_2()       (CHECK_TOUCH_2() && CHECK_CHANGE_2())

#define CHECK_DOUBLE_BOTH() (sensor_state & SENSE_DOUBLE_BOTH)
#define CHECK_LONG_BOTH()   (sensor_state & SENSE_LONG_BOTH)

#define CHECK_RANGE_SHORT() (sensor_state & SENSE_RANGE_SHORT)
#define CHECK_RANGE_MID()   (sensor_state & SENSE_RANGE_MID)
#define CHECK_RANGE_LONG()  (sensor_state & SENSE_RANGE_LONG)
#define CHECK_RANGE_MAX()   (sensor_state & SENSE_RANGE_MASENSOR_STATE)

#define CHECK_TOUCH_BOTH()  ((sensor_state & SENSE_TOUCH_ALL) == SENSE_TOUCH_ALL)
#define CHECK_CHANGE_BOTH() ((sensor_state & SENSE_CHANGE_ALL) == SENSE_CHANGE_ALL)

#define CHECK_TOUCH_NONE()  ((sensor_state & SENSE_TOUCH_ALL) == 0)
#define CHECK_CHANGE_NONE() ((sensor_state & SENSE_CHANGE_ALL) == 0)
#define CHECK_TOUCH_ANY()   (sensor_state & SENSE_TOUCH_ALL)
#define CHECK_CHANGE_ANY()  (sensor_state & SENSE_CHANGE_ALL)

// XXX: One-then-other required as the sense time is fine enough that
//      it often is triggered that way when trying to touch both.  This
//      could be improved by tracking the time for the individual sensors
//      and setting double-tap only if the delay is very short.
#define CHECK_TAP_BOTH(x)    (CHECK_TOUCH_BOTH(x) && CHECK_CHANGE_ANY(x))

void handle_sensors();

/***** Range finder *****/
#define PING_TRIG_PIN 9
#define PING_ECHO_PIN 6

#define PING_SHORT_CM 25
#define PING_MID_CM   50
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
#define CAP_DOUBLE_MS 750   /* Max milliseconds for a double-tap */
void sensor_cap_init(void);
void sensor_cap(void);


/* Sensor modes */
typedef void (*sensor_mode_t)(boolean entered, boolean exited);

void sensor_mode_basic_control(boolean entered, boolean exited);
void sensor_mode_poofer_control(boolean entered, boolean exited);
void sensor_mode_mode_control(boolean entered, boolean exited);

/***** Cube Light Connectivity ***********************************************/

extern RS485Socket rs485;
extern uint16_t my_address;

//#define ADDRESS_RECV_TEST  RS485_ADDR_ANY
#define ADDRESS_SOUND_UNIT  0x01
//#define ADDRESS_TRIANGLES  0x10

#define ADDRESS_POOFER_UNIT 0x41
#define ADDRESS_LIGHT_UNIT ADDRESS_POOFER_UNIT //0x41

void initializeConnect();

void sendByte(byte value, byte address);
void sendInt(int value, byte address);
void sendLong(long value, byte address);
void sendHMTLValue(uint16_t address, uint8_t offset, int value);
void sendHMTLBlink(uint16_t address, uint8_t output,
		   uint16_t on_period, uint32_t on_color,
		   uint16_t off_period, uint32_t off_color);
void sendHMTLTimedChange(uint16_t address, uint8_t output,
			 uint32_t change_period,
			 uint32_t start_color, uint32_t stop_color);
void sendHMTLSensorRequest(uint16_t address);

/***** Cube light modes *******************************************************/

/* Return the current mode value */
uint8_t get_current_mode(uint8_t place);

/* Set the mode */
void set_mode(uint8_t place, uint8_t new_mode);
void set_mode_to(uint8_t place, uint8_t new_mode);
void increment_mode(uint8_t place);
void restore_mode(uint8_t place);

#define PATTERN_DATA_SZ 16
typedef struct {
  uint32_t bgColor;
  uint32_t fgColor;
  uint32_t next_time;
  uint16_t periodms;
  union {
    uint8_t bytes[PATTERN_DATA_SZ];
    uint32_t u16s[PATTERN_DATA_SZ / sizeof (uint32_t)];
    uint32_t u32s[PATTERN_DATA_SZ / sizeof (uint16_t)];
  } data;
} pattern_args_t;

#define MAX_MODES 3
#define FINAL_MODE (MAX_MODES - 1)
extern pattern_args_t modeConfigs[MAX_MODES];

typedef void (*square_mode_t)(Square *squares, int size, pattern_args_t *arg);

extern square_mode_t modeFunctions[];
extern uint16_t modePeriods[];
extern square_mode_t followupFunctions[];
extern uint16_t followupPeriods[];

/* Index into the modeFunctions array */
#define MODE_NONE            (byte)-1
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
#define MODE_BAR_CIRCLE     11
#define MODE_CRAWL          12
#define MODE_BLINK_PATTERN  13
#define MODE_ORBIT_TEST     14
#define MODE_VECTORS        15
#define MODE_SIMPLE_LIFE    16
#define MODE_SOUND_TEST     17
#define MODE_SOUND_HMTL     18
#define MODE_STROBE         19

void squaresTestPattern(Square *squares, int size, pattern_args_t *arg);
void squaresSetupPattern(Square *squares, int size, pattern_args_t *arg);
void squaresRandomNeighbor(Square *squares, int size, pattern_args_t *arg);
void squaresCyclePattern(Square *squares, int size, pattern_args_t *arg);
void squaresCirclePattern(Square *squares, int size, pattern_args_t *arg);
void squaresFadeCycle(Square *squares, int size, pattern_args_t *arg);
void squaresAllOn(Square *squares, int size, pattern_args_t *arg);
void squaresCapResponse(Square *squares, int size, pattern_args_t *arg);
void squaresStaticNoise(Square *squares, int size, pattern_args_t *arg);
void squaresSwitchRandom(Square *squares, int size, pattern_args_t *arg);
void squaresBarCircle(Square *squares, int size, pattern_args_t *arg);
void squaresCrawl(Square *squares, int size, pattern_args_t *arg);
void squaresOrbitTest(Square *squares, int size, pattern_args_t *arg);
void squaresVectors(Square *squares, int size, pattern_args_t *arg);
void squaresSimpleLife(Square *squares, int size, pattern_args_t *arg);
void squaresSoundTest(Square *squares, int size, pattern_args_t *arg);
void squaresSoundHMTL(Square *squares, int size, pattern_args_t *arg);
void squaresStrobe(Square *squares, int size, pattern_args_t *arg);


void squaresLightCenter(Square *squares, int size, pattern_args_t *arg);
void squaresBlinkPattern(Square *squares, int size, pattern_args_t *arg);

#endif
