/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/
#ifndef CUBELIGHTS_H
#define CUBELIGHTS_H

/* ***** LED Driver config ***** */

#define MAX_VALUE 4095 // Max TLC pin value

#define NUM_LEDS 12
#define TLC_DEBUG_LED1 14
#define TLC_DEBUG_LED2 15

extern int16_t ledValues[];

/* ***** LED modes ***** */

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


/* ***** Sensor info ***** */

/* Capacitive side sensors */
#define NUM_SIDE_SENSORS 2
extern long side_values[];

/* Range finder */
extern uint16_t range_cm;

#endif
