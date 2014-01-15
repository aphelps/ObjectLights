#ifndef CUBE_CONFIG_H
#define CUBE_CONFIG_H

#define ADAM_CUBE   0
#define DEE_CUBE    1
#define PARENT_CUBE 2
#define SETH_CUBE   3

#define CUBE_NUMBER ADAM_CUBE

#define CAP_SENSOR_1 0
#define CAP_SENSOR_2 1

#define CAP_TOUCH_IRQ 2

// Touch sensor trigger and release values
#if CUBE_NUMBER == PARENT_CUBE
  #define CAP_SENSOR_1_TOUCH 6
  #define CAP_SENSOR_1_RELEASE 2
  #define CAP_SENSOR_2_TOUCH 6
  #define CAP_SENSOR_2_RELEASE 2
#elif CUBE_NUMBER == ADAM_CUBE
  #define CAP_SENSOR_1_TOUCH 9
  #define CAP_SENSOR_1_RELEASE 1
  #define CAP_SENSOR_2_TOUCH 9
  #define CAP_SENSOR_2_RELEASE 4
#else
  #define CAP_SENSOR_1_TOUCH 6
  #define CAP_SENSOR_1_RELEASE 2
  #define CAP_SENSOR_2_TOUCH 6
  #define CAP_SENSOR_2_RELEASE 2
#endif

// Some pixel strands may need to start after the initial LED
#if CUBE_NUMBER == ADAM_CUBE
  #define FIRST_LED 1
#else
  #define FIRST_LED 0
#endif

// Square layout

#endif
