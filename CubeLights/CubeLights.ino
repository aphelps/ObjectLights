/*******************************************************************************
 * 
 * 
 * XXX: Put a license here
 ******************************************************************************/


#define DEBUG_LEVEL 2
#include <Debug.h>

#include <Arduino.h>
#include <CapacitiveSensor.h>
#include <NewPing.h>
#include "Tlc5940.h"

#include "CubeLights.h"

#if NUM_TLCS != 1
  /* NUM_TLCS must be set to 1 in tlc_config.h */
  NUM_TLCS must equal 1;
#endif

/* TCL Pin values */
int16_t ledValues[NUM_LEDS] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
};

mode_function_t modeFunctions[] = {
  mode_set_all,          // MODE_SET_ALL
  mode_swap_one,         // MODE_SWAP_ONE
  mode_fade_one,         // MODE_FADE_ONE
  mode_count_up,         // MODE_COUNT_UP
  mode_flash_ordered,    // MODE_FLASH_ORDERED
  mode_random_fades,     // MODE_RANDOM_FADES
  mode_sense_distance,   // MODE_SENSE_DISTANCE
};

void * modeArguments[] = {
  (void *)MAX_VALUE,    // MODE_SET_ALL
  NULL,                 // MODE_SWAP_ONE
  NULL,                 // MODE_FADE_ONE
  NULL,                 // MODE_COUNT_UP
  NULL,                 // MODE_FLASH_ORDERED
  NULL,                 // MODE_RANDOM_FADES
  NULL,                 // MODE_SENSE_DISTANCE
};

#define INITIAL_VALUE 0

#define CAP_DELAY_MS 250
CapacitiveSensor side_sensors[NUM_SIDE_SENSORS] = {
  CapacitiveSensor(4,5),
  CapacitiveSensor(4,6),
};
long side_values[NUM_SIDE_SENSORS];
long side_min[NUM_SIDE_SENSORS];
long side_max[NUM_SIDE_SENSORS];

#define PING_TRIG 2
#define PING_ECHO 12
#define PING_MAX_CM 200
#define PING_DELAY_MS 250 /* Frequency (in ms) to trigger the range finder */
NewPing sonar(PING_TRIG, PING_ECHO, PING_MAX_CM);

#define PHOTO_PIN A0

/******************************************************************************
 * Initialization
 *****************************************************************************/
void setup()
{
  //Serial.begin(9600);
  Serial.begin(115200);

  for (int side = 0; side < NUM_SIDE_SENSORS; side++) {
    side_sensors[side].set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
    side_sensors[side].set_CS_Timeout_Millis(100);
    side_min[side] = 0xFFFF;
    side_max[side] = 0;
  }

  /* Initialize the LED drivers with all-off */ 
  Tlc.init(INITIAL_VALUE);

  /* Initialize the LED values */
  for (int led = 0; led < NUM_LEDS; led++) {
    ledValues[led] = INITIAL_VALUE;
  }

  randomSeed(analogRead(0));

  /* Initialize the range sensor */
//  pinMode(PING_TRIG, OUTPUT);
//  pinMode(PING_ECHO,INPUT);

  /* Turn on input pullup on analog photo pin */
  digitalWrite(PHOTO_PIN, HIGH); 
}

uint16_t range_cm = 10;
void check_ping()  // Interupt driven range finder
{
  if (sonar.check_timer()) {
    range_cm = sonar.ping_result / US_ROUNDTRIP_CM;
    DEBUG_PRINT(2, "Ping: ");
    DEBUG_PRINT(2, range_cm); // Ping returned, uS result in ping_result, convert to cm with US_ROUNDTRIP_CM.
    DEBUG_PRINT(2, "cm");
  }
}

void check_ping_2() // Non-interupt range finder
{
  range_cm = sonar.ping() / US_ROUNDTRIP_CM;
  DEBUG_PRINT(2, "Ping: ");
  DEBUG_PRINT(2, range_cm);
  DEBUG_PRINT(2, "cm - photo:");
  DEBUG_PRINT(2, "\n");
}

void range_finder_check(void) 
{
  static long nextPing = millis();
  long now = millis();
  if (now >= nextPing) {
    nextPing = now + PING_DELAY_MS;
    //sonar.ping_timer(check_ping);
    check_ping_2();
  }
}

void cap_sensor_check(void) 
{
  /* Determine if its time to perform a check */
  static long next_check = millis();
  long now = millis();
  if (now < next_check) return;
  next_check = now + CAP_DELAY_MS;
  
  /* Read the side sensors */
  long start = millis();
  long sense_delay;
  for (int side = 0; side < NUM_SIDE_SENSORS; side++) {
    //long value = side_sensors[side].capacitiveSensor(10);
    long value = side_sensors[side].capacitiveSensorRaw(1);
    if (value < side_min[side]) side_min[side] = value;
    if (value > side_max[side]) side_max[side] = value;

    side_values[side] = value;
//    side_values[side] = map(value,
//                            side_min[side], side_max[side],
//                            0, MAX_VALUE);
  }
  DEBUG_PRINT(2, "Cap: ");
  sense_delay = millis() - start;
  DEBUG_PRINT(2, sense_delay);        // check on performance in milliseconds
  DEBUG_PRINT(2, "\t");                    // tab character for debug windown spacing
  for (int side = 0; side < NUM_SIDE_SENSORS; side++) {
    DEBUG_PRINT(2, side_values[side]);
    DEBUG_PRINT(2, "-");
    DEBUG_PRINT(2, log(side_values[side]));
    DEBUG_PRINT(2, "    ");
  }
  DEBUG_PRINT(2, "\n");
}

#define PHOTO_THRESHOLD_LOW  85  // Turn light off when below this level
#define PHOTO_THRESHOLD_HIGH 100 // Turn light on when above this level
boolean photo_dark = false;
void photo_sensor_check(void) 
{
  int photo_value = analogRead(PHOTO_PIN);
  if (photo_value > PHOTO_THRESHOLD_HIGH) {
    photo_dark = false;
  } else if (photo_value < PHOTO_THRESHOLD_LOW) {
    photo_dark = false;
  }
//  DEBUG_VALUE(2, "Photo:", photo_value);
//  DEBUG_PRINT(2, "\n");
}


/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{
  void *mode_arg = NULL;

  cap_sensor_check();

//  range_finder_check();

  photo_sensor_check();

  int delay_period_ms;
  if (photo_dark) {
    /* Get the current mode */
    int mode = get_current_mode();

    /* Call the action function for the current mode */
    delay_period_ms = modeFunctions[mode](modeArguments[mode]);

    DEBUG_VALUE(3, "Mode:", mode);
  } else {
    /* When its light out then turn the lights off */
    delay_period_ms = mode_set_all(0);
  }

  DEBUG_VALUELN(3, "Per:", delay_period_ms);
  
  /* Wait for the specifided interval */
  delay(delay_period_ms);
}
