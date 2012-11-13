#ifndef UMBRELLASIGN
#define UMBRELLASIGN

/* TLC Output Values */
#define MAX_VALUE 4095
#define NUM_LEDS 28
extern uint16_t ledValues[];

/* Definition of sign modes */
typedef int (*mode_function_t)(void *arg);

/* Mode for lights */
#define MODE_EXAMPLE_CIRCULAR 0
#define MODE_EXAMPLE_FADES    1
#define MODE_ALL_ON           2
#define MODE_SWAP_ONE         3
#define MODE_FADE_ONE         4
#define MODE_TOTAL            5

/* Mode functions */
int mode_example_circular(void *arg);
int mode_example_fades(void *arg);
int mode_all_on(void *arg);
int mode_swap_one(void *arg);
int mode_fade_one(void *arg);

/* Return the current mode value */
int get_current_mode(void);

#endif
