/*
 * This code drive's Adam's Umbrella sign.
 *
 * Internal to the sign is an Arduino Nano v3 driving two serially connected
 * TLC5940 LED drivers for the individual pixels of the sign.
 */
#include <Arduino.h>
#include <CapacitiveSensor.h>
#include <NewPing.h>
#include "Tlc5940.h"

#include "ChristmasLights.h"

#if NUM_TLCS != 1
  /* NUM_TLCS must be set to 2 in tlc_config.h */
  NUM_TLCS must equal 1;
#endif

/* Array to map the light positions in the sign to the LED driver pins */
int8_t signToIndex[] =
{
  19, 10, 8,  18, 12, -1, -1, -1,
  -1, -1, 11, 16,  7,  9, 17, -1,
  -1, -1, -1, 20, 22, 21,  4, -1,
  -1, -1, -1, -1, 23,  5, 13,  3,
  -1, -1, -1, -1, -1, 25,  6,  1,
  -1, -1, 26, -1, -1, -1, 15, 24,
  -1,  0, -1, -1, -1, -1, -1, 14,
  27, -1, -1, -1, -1, -1, -1,  2,
};

/* Mapping of LEDs into rows along the diaginal axis */
int8_t signRows[] =
{
   5,  4,  3,  2,  1, -1, -1, -1,
  -1, -1,  4,  3,  2,  1,  0, -1,
  -1, -1, -1,  4,  3,  2,  1, -1,
  -1, -1, -1, -1,  4,  3,  2,  1,
  -1, -1, -1, -1, -1,  4,  3,  2,
  -1, -1,  6, -1, -1, -1,  4,  3,
  -1,  7, -1, -1, -1, -1, -1,  4,
   8, -1, -1, -1, -1, -1, -1,  5,
};

// XXX - A row (or 2) is skipped because its blank

int8_t ledRow[NUM_LEDS];
int16_t rowValues[MAX_ROW] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE,
};

/* Mapping of LEDs into columns along the diaginal axis */
int8_t signColumns[] =
{
   0,  1,  2,  3,  4, -1, -1, -1,
  -1, -1,  3,  4,  5,  6,  7, -1,
  -1, -1, -1,  5,  6,  7,  8, -1,
  -1, -1, -1, -1,  7,  8,  9, 10,
  -1, -1, -1, -1, -1,  9, 10, 11,
  -1, -1,  7, -1, -1, -1, 11, 12,
  -1,  7, -1, -1, -1, -1, -1, 13,
   7, -1, -1, -1, -1, -1, -1, 14,
};

int8_t ledColumn[NUM_LEDS];
int16_t columnValues[MAX_COLUMN] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE,
};

/* TCL Pin values */
int16_t ledValues[NUM_LEDS] = {
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
  MAX_VALUE, MAX_VALUE, MAX_VALUE, MAX_VALUE,
};

mode_function_t modeFunctions[] = {
  mode_example_circular, // MODE_EXAMPLE_CIRCULAR
  mode_example_fades,    // MODE_EXAMPLE_FADES
  mode_all_on,           // MODE_ALL_ON
  mode_swap_one,         // MODE_SWAP_ONE
  mode_fade_one,         // MODE_FADE_ONE
  mode_fade_row,         // MODE_FADE_ROW
  mode_fade_column,      // MODE_FADE_COLUMN
  mode_count_up,         // MODE_COUNT_UP
  mode_flash_ordered,    // MODE_FLASH_ORDERED
  mode_cross_fade,       // MODE_CROSS_FADE
  mode_random_fades,     // MODE_RANDOM_FADES
  mode_sense_distance,
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

  /* Populate the ledRowArray */
  DEBUG_PRINT("ledRowArray:\n");
  for (uint32_t i = 0; i < sizeof (signRows); i++) {
    int8_t led = signToIndex[i];
    if (led < 0) {
      DEBUG_PRINT("X/X");
    } else {
      ledRow[led] = signRows[i];
      DEBUG_PRINT(led);
      DEBUG_PRINT("/");
      DEBUG_PRINT(ledRow[led]);
    }
    if (i % 8 == 7) {
      DEBUG_PRINT("\n");
    } else {
      DEBUG_PRINT(", ");
    }
  }

  DEBUG_PRINT("ledRow:");
  for (int i = 0; i < NUM_LEDS; i++) {
    DEBUG_PRINT(ledRow[i]);
    DEBUG_PRINT(", ");
  }
  DEBUG_PRINT("\n");


   /* Populate the ledColumnArray */
  DEBUG_PRINT("ledColumnArray:\n");
  for (uint32_t i = 0; i < sizeof (signColumns); i++) {
    int8_t led = signToIndex[i];
    if (led < 0) {
      DEBUG_PRINT("X/X");
    } else {
      ledColumn[led] = signColumns[i];
      DEBUG_PRINT(led);
      DEBUG_PRINT("/");
      DEBUG_PRINT(ledColumn[led]);
    }
    if (i % 8 == 7) {
      DEBUG_PRINT("\n");
    } else {
      DEBUG_PRINT(", ");
    }
  }

  DEBUG_PRINT("ledColumn:");
  for (int i = 0; i < NUM_LEDS; i++) {
    DEBUG_PRINT(ledColumn[i]);
    DEBUG_PRINT(", ");
  }
  DEBUG_PRINT("\n");

  /* Initialize the range sensor */
//  pinMode(PING_TRIG, OUTPUT);
//  pinMode(PING_ECHO,INPUT);

  /* Turn on input pullup on analog photo pin */
  digitalWrite(PHOTO_PIN, HIGH); 
}

uint16_t range_cm = 10;
void check_ping() 
{
  if (sonar.check_timer()) {
    range_cm = sonar.ping_result / US_ROUNDTRIP_CM;
    Serial.print("Ping: ");
    Serial.print(range_cm); // Ping returned, uS result in ping_result, convert to cm with US_ROUNDTRIP_CM.
    Serial.println("cm");
  }
}

void check_ping_2() 
{
  range_cm = sonar.ping() / US_ROUNDTRIP_CM;
  Serial.print("Ping: ");
  Serial.print(range_cm);
  Serial.print("cm - photo:");

  Serial.print(analogRead(PHOTO_PIN));
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
    side_values[side] = map(value,
                            side_min[side], side_max[side],
                            0, MAX_VALUE);
  }
  Serial.print("Cap: ");
  sense_delay = millis() - start;
  Serial.print(sense_delay);        // check on performance in milliseconds
  Serial.print("\t");                    // tab character for debug windown spacing
  for (int side = 0; side < NUM_SIDE_SENSORS; side++) {
    Serial.print(side_values[side]);
    Serial.print("-");
    Serial.print(log(side_values[side]));
    Serial.print("    ");
  }
}

/******************************************************************************
 * Action loop
 *****************************************************************************/
void loop()
{

//  cap_sensor_check();

//  range_finder_check();

  void *mode_arg = NULL;
  if (analogRead(PHOTO_PIN) < 85) {
    set_current_mode(MODE_ALL_ON);
    mode_arg = (void *)1;
  } else {
    restore_current_mode();
    mode_arg = NULL;
  }

  /* Get the current mode */
  int mode = get_current_mode();

  /* Call the action function for the current mode */
  int delay_period = modeFunctions[mode](mode_arg);

  Serial.print("Mode:");
  Serial.print(mode);
  Serial.print("-");
  Serial.print(delay_period);

  Serial.print("\n");
  
  /* Wait for the specifided interval */
  delay(delay_period);
}

