#ifndef CUBE_CONFIG_H
#define CUBE_CONFIG_H

#define NUM_SQUARES 6

#define ADAM_CUBE   0
#define DEE_CUBE    1
#define PARENT_CUBE 2
#define SETH_CUBE   3
#define OTHER_CUBE  4

#define BIG_CUBE    5

#ifndef CUBE_NUMBER
#define CUBE_NUMBER ADAM_CUBE
#endif

#define CUBE_VERSION 1

// Touch sensor trigger and release values
#if CUBE_NUMBER == BIG_CUBE
#warning CUBE_NUMBER IS SET TO BIG_CUBE!!!
  #define CAP_SENSOR_1 11
  #define CAP_SENSOR_2 9
#else
  #define CAP_SENSOR_1 0
  #define CAP_SENSOR_2 1
#endif

// Some pixel strands may need to start after the initial LED
#if CUBE_NUMBER == ADAM_CUBE
  #define FIRST_LED 1
#else
  #define FIRST_LED 0
#endif

// Square layout

#endif
