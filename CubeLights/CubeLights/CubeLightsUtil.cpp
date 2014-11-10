/*******************************************************************************
 * Author: Adam Phelps
 * License: Create Commons Attribution-Non-Commercial
 * Copyright: 2014
 ******************************************************************************/

#include <Arduino.h>

#define DEBUG_LEVEL DEBUG_HIGH
#include "Debug.h"

#include "CubeConfig.h"
#include "CubeLights.h"

void initializePins() {
  /* Turn on input pullup on analog light sensor pin */
  digitalWrite(PHOTO_PIN, HIGH);
}

/*******************************************************************************
 * Square light patterns
 */

void setAllSquares(Square *squares, int size, uint32_t color) {
  for (int tri = 0; tri < size; tri++) {
    squares[tri].setColor(color);
    squares[tri].mark = 0;
  }
}

void clearSquares(Square *squares, int size) {
  setAllSquares(squares, size, 0);
}

void randomSquares(Square *squares, int size) {
  for (int tri = 0; tri < size; tri++) {
    byte red = random(0, 16);
    byte green = random(0, 16);
    byte blue = random(0, 16);
    red = red * red;
    green = green * green;
    blue = blue * blue;

    squares[tri].setColor(red, green, blue);
  }
}

void wheelSquares(Square *squares, int size) {
  for (int tri = 0; tri < size; tri++) {
    squares[tri].setColor(pixel_wheel(map(tri, 0, size - 1, 0, 255)));
  }
}

void binarySquares(Square *squares, int size, uint32_t color, int thresh) 
{
  for (int tri = 0; tri < size; tri++) {
    boolean set = (random(0, 100) > thresh);
    if (set) squares[tri].setColor(color);
    else squares[tri].setColor(0);
  }
}

void randomBinarySquares(Square *squares, int size, byte color, int thresh) 
{
  for (int tri = 0; tri < size; tri++) {
    boolean red = (random(0, 100) > thresh);
    boolean green = (random(0, 100) > thresh);
    boolean blue = (random(0, 100) > thresh);

    squares[tri].setColor((red ? color : 0),
			    (blue ? color : 0),
			    (green ? color : 0));   
  }
}

void incrementMarkAll(Square *squares, int size, char incr) {
  for (int tri = 0; tri < size; tri++) {
    int value = squares[tri].mark + incr;
    if (value < 0) squares[tri].mark = 0;
    else if (value > 255) squares[tri].mark = 255;
    else squares[tri].mark = value;
  }
}

/* Adjusted every led by the indicated amount */
void incrementAll(Square *squares, int size,
                  char r, char g, char b) {
  for (int tri = 0; tri < size; tri++) {
      for (byte c = 0; c < 3; c++) {
	int red = squares[tri].leds[c].red;
 	int green = squares[tri].leds[c].green;
	int blue = squares[tri].leds[c].blue;

	red = red + r;
	if (red < 0) red = 0;
	else if (red > 255) red = 255;

	green = green + g;
	if (green < 0) green = 0;
	else if (green > 255) green = 255;

	blue = blue + b;
	if (blue < 0) blue = 0;
	else if (blue > 255) blue = 255;

	squares[tri].setColor(c, (byte)red, (byte)green, (byte)blue);
      }
  }
}



/******************************************************************************
 * Square Patterns
 */

/* This iterates through the squares, lighting the ones with leds */
void squaresTestPattern(Square *squares, int size,
			  pattern_args_t *arg) {
  static int current = 0;

  if (arg->next_time == 0) {
    current = 0;
    arg->next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    /* Clear the color of the previous square and its edges*/
    squares[current % size].setColor(0, 0, 0);
    for (byte edge = 0; edge < 3; edge++) {
      squares[current % size].edges[edge]->setColor(0, 0, 0);
    }

    current = (current + 1) % size;

    /* Set the color on the new square and its edges */
    squares[current % size].setColor(255, 0, 0);
    squares[current % size].edges[0]->setColor(00, 01, 00);
    squares[current % size].edges[1]->setColor(00, 00, 01);
    squares[current % size].edges[2]->setColor(01, 00, 01);
  }
}

/*
 * This setup pattern iterates through the squares, iterating through the LEDs
 * on each one.
 */
void squaresSetupPattern(Square *squares, int size,
			  pattern_args_t *arg) {
  static byte current = 0;
  static byte led = 0;

  if (arg->next_time == 0) {
    current = 0;
    led = 0;
    arg->next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    /* Clear the color of the previous square */
    squares[current % size].setColor(0, 0, 0);

    if (led >= Square::NUM_LEDS) {
      current = (current + 1) % size;
      led = 0;
    }

    if (led == 0) {
      squares[current % size].setColor(led, pixel_secondary(map(current, 0, 5, 0, 255)));
    } else {
      squares[current % size].setColor(led, 255, 255, 255);
    }

    led++;
  }
}


/* This iterates through the squares, lighting the ones with leds */
void squaresRandomNeighbor(Square *squares, int size,
			     pattern_args_t *arg) {
  static Square *current = &squares[0];

  if (arg->next_time == 0) {
    current = &squares[0];
    arg->next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    /* Clear the color of the previous square */
    current->setColor(0, 0, 0);
    for (byte edge = 0; edge < 3; edge++) {
      current->edges[edge]->setColor(0, 0, 0);
    }

    /* Choose the next square */
    byte edge;
    do {
      edge = random(0, Square::NUM_EDGES);
    } while (!current->edges[edge]->hasLeds);
    current = current->edges[edge];

    /* Set the color on the new square */
    current->setColor(arg->fgColor);
    for (byte edge = 0; edge < 3; edge++) {
      current->edges[edge]->setColor(fadeTowards(arg->fgColor, 0, 95));
    }
  }

}

void squaresCyclePattern(Square *squares, int size,
			  pattern_args_t *arg) {
  static byte phase = 0;

  if (arg->next_time == 0) {
    phase = 0;
    arg->next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    for (int i = 0; i < size; i++) {
      for (int j = 0; j < Square::NUM_LEDS; j++) {
	switch (phase % 2) {
	case 0: {
	  switch (j) {
          case 0: case 2: case 6: case 8: case 4:
	    squares[i].setColor(j, arg->fgColor);
	    break;
	  default:
	    squares[i].setColor(j, arg->bgColor);
	    break;
	  }
	  break;
	}
	case 1: {
	  switch (j) {
	  case 1: case 3: case 5: case 7:
	    squares[i].setColor(j, arg->fgColor);
	    break;
	  default:
	    squares[i].setColor(j, arg->bgColor);
	    break;
	  }
	  break;
	}
	}
      }
    }

    phase++;
  }
}


void squaresCirclePattern(Square *squares, int size,
			  pattern_args_t *arg) {
  static byte phase = 0;

  if (arg->next_time == 0) {
    phase = 0;
    arg->next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    for (int i = 0; i < size; i++) {
      switch (phase % 8) {
      case 0: {
	squares[i].setColor(3, arg->bgColor);
	squares[i].setColor(0, arg->fgColor);
	break;
      }
      case 1: {
	squares[i].setColor(0, arg->bgColor);
	squares[i].setColor(1, arg->fgColor);
	break;
      }
      case 2: {
	squares[i].setColor(1, arg->bgColor);
	squares[i].setColor(2, arg->fgColor);
	break;
      }
      case 3: {
	squares[i].setColor(2, arg->bgColor);
	squares[i].setColor(5, arg->fgColor);
	break;
      }
      case 4: {
	squares[i].setColor(5, arg->bgColor);
	squares[i].setColor(8, arg->fgColor);
	break;
      }
      case 5: {
	squares[i].setColor(8, arg->bgColor);
	squares[i].setColor(7, arg->fgColor);
	break;
      }
      case 6: {
	squares[i].setColor(7, arg->bgColor);
	squares[i].setColor(6, arg->fgColor);
	break;
      }
      case 7: {
	squares[i].setColor(6, arg->bgColor);
	squares[i].setColor(3, arg->fgColor);
	break;
      }
      }
    }

    phase++;
  }
}

void squaresFadeCycle(Square *squares, int size,
			  pattern_args_t *arg) {
  static long phase_start = 0;
  long max_phase = 15 * 1000;
  static boolean fadeup = true;

  unsigned long now = millis();

  if (arg->next_time == 0) {
    phase_start = now;
    arg->next_time = now;
    clearSquares(squares, size);
  }

  if (now > arg->next_time) {
    long phase = now - phase_start;

    arg->next_time += arg->periodms;

    for (int i = 0; i < size; i++) {
      if (fadeup) {
	squares[i].setColor(
			    fadeTowards(
					arg->bgColor,
					arg->fgColor,
					map(phase, 0, max_phase, 0, 100)
					)
			    );
      } else {
	  squares[i].setColor(
			      fadeTowards(
					  arg->fgColor,
					  arg->bgColor,
					  map(phase, 0, max_phase, 0, 100)
					  )
			      );
      }
    }

    phase++;
    if (phase > max_phase) {
      fadeup = !fadeup;
      phase_start = now;
    }
  }
}

void squaresAllOn(Square *squares, int size,
		  pattern_args_t *arg) {
  if (arg->next_time == 0) {
    arg->next_time = millis();
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;
    setAllSquares(squares, size, arg->fgColor);
  }
}

/*
 * This mode reacts to the capacitive touch sensors
 */
void squaresStaticNoise(Square *squares, int size,
			pattern_args_t *arg) {
  if (arg->next_time == 0) {
    arg->next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    /* Set the leds randomly to on off in white */
    for (int square = 0; square < size; square++) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	if (random(0, 100) < 60) {
	  squares[square].setColor(led, arg->bgColor);
	} else {
	  int facter = random(0, 7);
	  byte red = pixel_red(arg->fgColor) >> facter;
	  byte green = pixel_green(arg->fgColor) >> facter;
	  byte blue = pixel_blue(arg->fgColor) >> facter;
	  //DEBUG_VALUE(DEBUG_HIGH, "r=", red);
	  //DEBUG_VALUE(DEBUG_HIGH, " g=", green);
	  //DEBUG_VALUELN(DEBUG_HIGH, " b=", blue);
	  squares[square].setColor(led, red, green, blue);
	}
      }
    }
  }
}

void squaresSwitchRandom(Square *squares, int size,
			pattern_args_t *arg) {
  if (arg->next_time == 0) {
    arg->next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    byte square = random(0, 5);
    byte led = random(0, Square::NUM_LEDS);

    if (squares[square].getColor(led) != arg->bgColor) {
      squares[square].setColor(led, arg->bgColor);
    } else {
      squares[square].setColor(led, arg->fgColor);
    }
  }
}


/*
 * This mode reacts to the capacitive touch sensors
 */
void squaresCapResponse(Square *squares, int size,
			pattern_args_t *arg) {
  
}


void squaresBarCircle(Square *squares, int size,
		      pattern_args_t *arg) {
  static Square *face = NULL;
  static byte current_bar = 0;

  if (arg->next_time == 0) {
    face = &squares[0];
    arg->next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    Square *top = face->edges[Square::TOP];
    top->setColorEdge(top->matchEdge(face), arg->bgColor);
    face->setColorColumn(current_bar, arg->bgColor);

    current_bar = (current_bar + 1) % 3;
    if (current_bar == 0) {
      face = face->edges[Square::RIGHT];
      top = face->edges[Square::TOP];
    }

    top->setColorEdge(top->matchEdge(face), arg->fgColor);
    face->setColorColumn(current_bar, arg->fgColor);
  }
}

/*
 * 
 */
void squaresCrawl(Square *squares, int size,
		  pattern_args_t *arg) {
  static Square *face = NULL;
  static byte led = 0;
  static byte color = 0;

  if (arg->next_time == 0) {
    face = &squares[0];
    led = 0;
    arg->next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > arg->next_time) {
    int timeincrement = arg->periodms;
    if (CHECK_TOUCH_1())
      timeincrement = timeincrement / 2;
    if (CHECK_TOUCH_2())
      timeincrement = timeincrement / 4;
    arg->next_time += timeincrement;

    face->setColor(led, pixel_wheel(color++));

    byte newled = led;
    Square *newface = face;

    byte mode = random(0, 2);
    switch (mode) {
    case 0: {
      do {
	if (led == Square::CENTER) break;
	newface = face->edges[random(0, Square::NUM_EDGES)];
	//	if (newface->hasLeds) {
	  newled = newface->matchLED(face, led);
	  //}
      } while(newled == (byte)-1);
      break;
    }
    case 1: {
      newled = random(0, 8);
      break;
    }
    }
    led = newled;
    face = newface;

    DEBUG_VALUE(DEBUG_TRACE, "crawl: mode=", mode);
    DEBUG_VALUE(DEBUG_TRACE, " face=", face->id);
    DEBUG_VALUELN(DEBUG_TRACE, " led=", led);
 
    face->setColor(led, arg->bgColor);
  }
}

void squaresOrbitTest(Square *squares, int size,
		  pattern_args_t *arg) {
  static Square *face = NULL;
  static Square *prevface = NULL;
  static byte led = 0;

  static Square *startface = NULL;
  static byte startled = 0;
  static byte direction = 0;

  boolean setup = false;

  if (arg->next_time == 0) {
    startface = &squares[0];
    startled = 7;
    direction = Square::BOTTOM;
    arg->next_time = millis();
    setup = true;
  }

  if (CHECK_TAP_BOTH()) {
    direction = (direction + 1) % Square::NUM_EDGES;
    setup = true;
  } else {
    if (CHECK_TAP_1()) {
      startface = &squares[(startface->id + 1) % size];
      setup = true;
    }
    if (CHECK_TAP_2()) {
      startled = (startled + 1) % Square::NUM_LEDS;
      setup = true;
    }
  }

  if (setup) {
    face = startface;
    prevface = face->edges[direction];
    led = startled;
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;

    face->setColor(led, arg->bgColor);

    uint16_t next = face->ledAwayFrom(prevface, led);
    led = LED_FROM_COMBO(next);

    if (FACE_FROM_COMBO(next) != face->id) {
      prevface = face;
      face = &squares[FACE_FROM_COMBO(next)];
    }
    face->setColor(led, arg->fgColor);
    startface->setColor(startled, pixel_color(255, 0, 0));
    DEBUG_VALUE(DEBUG_TRACE, "Orbit: ", face->id);
    DEBUG_VALUELN(DEBUG_TRACE, ":", led);
  }
}

/*
 * Trace vectors around the cube
 */
typedef struct {
  uint16_t led_in_face;
  byte direction;
  byte length;
} vector_t;

vector_t followVector(vector_t vector, Square *squares) {
  // Get the current face
  Square *face = &squares[FACE_FROM_COMBO(vector.led_in_face)];

  // Get the next face and led along the vector
  uint16_t next = face->ledTowards(LED_FROM_COMBO(vector.led_in_face),
				   vector.direction);
  Square *next_face = &squares[FACE_FROM_COMBO(next)];
  vector.led_in_face = next;
  if (face != next_face) {
    vector.direction = REV_DIRECTION(next_face->matchEdge(face));
  }
  return vector;
}

#define MAX_VECTORS (PATTERN_DATA_SZ / sizeof (vector_t))
#define RESET_PERIOD 1*60*1000
void squaresVectors(Square *squares, int size,
		    pattern_args_t *arg) {
  vector_t *vectors = (vector_t *)&arg->data.bytes;
  static byte color_index = 0;
  static byte num_vectors;
  static unsigned long prev_reset = 0;

  unsigned long now = millis();

  if ((arg->next_time == 0) ||
      (CHECK_TAP_1()) ||
      (CHECK_TAP_2()) ||
      (unsigned long)(now - prev_reset) >  (unsigned long)RESET_PERIOD) {

    num_vectors = 1 + random(MAX_VECTORS);
    for (int v = 0; v < num_vectors; v++) {
      vectors[v].led_in_face = FACE_AND_LED(
				      random(0, size),            // Face
				      random(0, Square::NUM_LEDS) // LED
				      );
      vectors[v].direction = random(0, Square::NUM_EDGES);
      vectors[v].length = random(2, 6);
    }
    setAllSquares(squares, size, arg->bgColor);
    arg->next_time = 0;
    DEBUG_VALUE(DEBUG_HIGH, "Num=", num_vectors);
    DEBUG_VALUELN(DEBUG_HIGH, " reset=", prev_reset);
    prev_reset = now;
    arg->next_time = millis();
  }

  if (now > arg->next_time) {
    arg->next_time += arg->periodms;

    for (byte v = 0; v < num_vectors; v++) {
      vector_t next_vector = followVector(vectors[v], squares);

      Square *face = &squares[FACE_FROM_COMBO(vectors[v].led_in_face)];
      Square *next_face = &squares[FACE_FROM_COMBO(next_vector.led_in_face)];

      // Set the new led
      next_face->setColor(LED_FROM_COMBO(next_vector.led_in_face), 
			  pixel_wheel(color_index));
    
      // Iterate backwards from the vector head to set colors
      vector_t reverse_vector = vectors[v];
      reverse_vector.direction = REV_DIRECTION(vectors[v].direction);
      Square *curr_face = face;
      for (byte i = 1; i <reverse_vector.length; i++) {
	curr_face->setColor(LED_FROM_COMBO(reverse_vector.led_in_face), 
			    pixel_wheel(color_index - 10 * i));
	reverse_vector = followVector(reverse_vector, squares);
	curr_face = &squares[FACE_FROM_COMBO(reverse_vector.led_in_face)];
      }
      // Clear the tail
      curr_face->setColor(LED_FROM_COMBO(reverse_vector.led_in_face), 
			  arg->bgColor);

      // Update the vector
      vectors[v] = next_vector;

      DEBUG_VALUE(DEBUG_TRACE, "Curr=", FACE_FROM_COMBO(vectors[v].led_in_face));
      DEBUG_VALUE(DEBUG_TRACE, ",", LED_FROM_COMBO(vectors[v].led_in_face));
      DEBUG_VALUELN(DEBUG_TRACE, "-", vectors[v].direction);
    }

    // Update the color
    color_index++;
  }
}

/* 
 * Set the center LED of each square to the indicated color
 */
#define SIMPLE_LIFE_RESET  0
#define SIMPLE_LIFE_SPLASH 1
void squaresSimpleLife(Square *squares, int size,
		       pattern_args_t *arg) {
  unsigned long now = millis();

  if (arg->next_time == 0) {
    arg->next_time = now;
    arg->data.u32s[SIMPLE_LIFE_SPLASH] = now;

    setAllSquares(squares, size, arg->bgColor);
    binarySquares(squares, size, arg->fgColor, 10);
  }

  if (arg->data.u32s[SIMPLE_LIFE_RESET]) {
    binarySquares(squares, size, arg->fgColor, random(0, 100));
  }

  if (CHECK_TOUCH_BOTH()) {
    for (byte face = 0; face < size; face++) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	if (squares[face].getColor(led) != arg->bgColor) {
	  squares[face].setColor(led, pixel_color(255, 255, 255));
	}
      }
    }
  } else {
    if (CHECK_TAP_1()) {
      arg->data.u32s[SIMPLE_LIFE_SPLASH] = 0;
    }
  }

  if (now > arg->next_time) {
    arg->next_time += arg->periodms;

    // Build neighbor counts
    byte neighbors[size][Square::NUM_LEDS];

    for (byte face = 0; face < size; face++) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	neighbors[face][led] = 0;
	for (byte direction = 0; direction < Square::NUM_EDGES; direction++) {
	  uint16_t l = squares[face].ledTowards(led, direction);
	  uint32_t c = squares[FACE_FROM_COMBO(l)].getColor(LED_FROM_COMBO(l));
	  if (c != arg->bgColor) {
	    neighbors[face][led]++;
	    squares[face].setColor(led, c);
	  }
	}
      }
    }

    byte count = 0;
    for (byte face = 0; face < size; face++) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	switch (neighbors[face][led]) {
	case 1:
	  count++;
	  break;
	default:
	  squares[face].setColor(led, arg->bgColor);
	  break;
	}
      }
    }

    if (now - arg->data.u32s[SIMPLE_LIFE_SPLASH] > 10000) {
      byte face = random(size - 1);
      byte led = random(Square::NUM_LEDS);
      byte c = random((byte)-1);
      squares[face].setColor(led, pixel_wheel(c));

      arg->data.u32s[SIMPLE_LIFE_SPLASH] = now;

      DEBUG_VALUE(DEBUG_HIGH, "Splash: ", face);
      DEBUG_VALUE(DEBUG_HIGH, ", ", led);
      DEBUG_VALUELN(DEBUG_HIGH, ", ", c);
    }

    if (count == 0) {
      arg->data.u32s[SIMPLE_LIFE_RESET] = 1;
      arg->next_time += 2000;
    } else {
      arg->data.u32s[SIMPLE_LIFE_RESET] = 0;
    }
  } else {
#if 0
    // Fade up to maximum brightness
    int elapsed_percent = ((arg->next_time - now) / arg->periodms) * 100;
    for (byte face = 0; face < size; face++) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	if (squares[face].getColor(led) != arg->bgColor) {
	  squares[face].setColor(led, fadeTowards(arg->bgColor, 
						  arg->fgColor,
						  elapsed_percent));
	}
      }
    }
#endif
  }
}

/*
 * Example mode to fetch sound samples from a remote module
 */
//#define SOUND_LEVELED
#define SOUND_TEST_TIMEOUT 500 // Max milliseconds to wait on a response
void squaresSoundTest(Square *squares, int size, pattern_args_t *arg) {
  static unsigned long lastSend = 0;

  if (arg->next_time == 0) {
    setAllSquares(squares, size, arg->bgColor);
  }

  if ((millis() > arg->next_time) && (lastSend == 0)) {
    // Send the data request
    arg->next_time += arg->periodms;

#ifdef SOUND_LEVELED
    sendByte('L', ADDRESS_SOUND_UNIT);
#else
    sendByte('C', ADDRESS_SOUND_UNIT);
#endif
    lastSend = millis();

    DEBUG_PRINT(DEBUG_TRACE, "SoundTest: Sent request...");
  } else if (lastSend != 0) {
    // Check for a response
    unsigned long elapsed = millis() - lastSend;
    unsigned int msglen;
    const byte *data = rs485.getMsg(RS485_ADDR_ANY, &msglen);
    if (data != NULL) {
      // Data should be an array of 8 uint16_t
      DEBUG_PRINT(DEBUG_TRACE, " value:");
#ifdef SOUND_LEVELED
      uint8_t *valptr = (uint8_t *)data;
#else
      uint16_t *valptr = (uint16_t *)data;
#endif
      byte face = 0;
      byte col = 0;
      uint32_t total = 0;
      while ((unsigned int)valptr - (unsigned int)data < msglen) {
#ifdef SOUND_LEVELED
	uint8_t val = *valptr;
#else
	uint16_t val = *valptr;
#endif
	DEBUG_HEXVAL(DEBUG_TRACE, " ", val);

	/* Shift the new value into each column */
	uint32_t newcolor;
	if (val) {
	  byte heat = (val > 15 ? 255 : val * val);
	  newcolor = pixel_heat(heat);
	} else {
	  newcolor = 0;
	}
	squares[face].shiftColumnDown(col % Square::SQUARE_LED_COLS, newcolor);

	col++;
	if (col % Square::SQUARE_LED_COLS == 0) {
	  face++;
	}

	total += val;

	valptr++;
      }
      DEBUG_VALUE(DEBUG_TRACE, " Elapsed:", elapsed);

      /* Set the top to the average */
      total = total / col;

      byte heat = total > 15 ? 255 : total * total;

      squares[CUBE_TOP].setColor(pixel_heat(heat));
      DEBUG_VALUE(DEBUG_TRACE, " avg:", total);
      DEBUG_PRINT_END();

#if 0
      // XXX - Trigger if over
      if (heat > 250) {
	static unsigned long last_send = 0;
	if (millis() - last_send > 100) {
	  sendHMTLTimedChange(ADDRESS_POOFER_UNIT, 2,
			      250, 0xFFFFFFFF, 0);
	  last_send = millis();
	}
      }
#endif

      lastSend = 0; // Reset the lastSend time so another request can be sent
    } else {
      if (elapsed > (unsigned long)SOUND_TEST_TIMEOUT) {
	DEBUG_VALUELN(DEBUG_HIGH, "SoundTest: No response after ", elapsed);
	lastSend = 0; // Reset the lastSend time so another request can be sent
      }
    }
  }
}

/*
 * Example mode to fetch sound samples from a remote module
 */
#define SOUND_TEST_TIMEOUT 500 // Max milliseconds to wait on a response
void squaresSoundTest2(Square *squares, int size, pattern_args_t *arg) {
  static unsigned long lastSend = 0;

  if (arg->next_time == 0) {
    setAllSquares(squares, size, arg->bgColor);
  }

  if ((millis() > arg->next_time) && (lastSend == 0)) {
    // Send the data request
    arg->next_time += arg->periodms;

    sendByte('S', ADDRESS_SOUND_UNIT);
    lastSend = millis();

    DEBUG_PRINT(DEBUG_HIGH, "SoundTest: Sent request...");
  } else if (lastSend != 0) {
    // Check for a response
    unsigned long elapsed = millis() - lastSend;
    unsigned int msglen;
    const byte *data = rs485.getMsg(RS485_ADDR_ANY, &msglen);
    if (data != NULL) {
      // Data should be an array of 8 uint16_t
      DEBUG_PRINT(DEBUG_HIGH, " value:");
      uint16_t *valptr = (uint16_t *)data;

      byte face = 0;
      byte led = 0;
      while ((unsigned int)valptr - (unsigned int)data < msglen) {
	uint16_t val = *valptr;

	DEBUG_HEXVAL(DEBUG_HIGH, " ", val);

	byte heat = (val > 15 ? 255 : val * val);
	uint32_t newcolor = pixel_heat(heat);

	squares[face].setColor(led, newcolor);

	led++;
	if (led == Square::NUM_LEDS) {
	  led = 0;
	  face++;
	}
	if (face > size)
	  break;

	valptr++;
      }
      DEBUG_VALUELN(DEBUG_HIGH, " Elapsed:", elapsed);

      lastSend = 0; // Reset the lastSend time so another request can be sent
    } else {
      if (elapsed > (unsigned long)SOUND_TEST_TIMEOUT) {
	DEBUG_VALUELN(DEBUG_HIGH, "SoundTest: No response after ", elapsed);
	lastSend = 0; // Reset the lastSend time so another request can be sent
      }
    }
  }
}

/******************************************************************************
 * Followup functions
 */

/* 
 * Set the center LED of each square to the indicated color
 */
void squaresLightCenter(Square *squares, int size,
			pattern_args_t *arg) {
  if (arg->next_time == 0) {
    arg->next_time = millis();
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;
    for (int square = 0; square < size; square++) {
      squares[square].setColor(4, arg->fgColor);
    }
  }
}

void squaresBlinkPattern(Square *squares, int size,
			 pattern_args_t *arg) {
  static boolean *on = (boolean *)&arg->data.u32s[1];

  if (arg->next_time == 0) {
    arg->next_time = millis();
    *on = false;
  }

  if (millis() > arg->next_time) {
    arg->next_time += arg->periodms;
    *on = !(*on);
  }

  uint32_t color;
  if (*on) color = arg->fgColor;
  else color = arg->bgColor;

  byte face_mask = FACES_FROM_MASK(arg->data.u32s[0]);
  uint32_t led_mask = LEDS_FROM_MASK(arg->data.u32s[0]);
  for (byte face = 0; face < size; face++) {
    if (face_mask & (1 << face)) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	if (led_mask & (1 << led)) {
	  squares[face].setColor(led, color);
	}
      }
    }
  }

}

/*
 * This implements a typical strobe effect
 */
struct strobe_args {
  boolean on;
  uint32_t next_sense;
  uint16_t default_period;
  uint16_t on_period;
  uint16_t off_period;
};
//#define STROBE_PROGRAM
void squaresStrobe(Square *squares, int size,
		   pattern_args_t *arg) {
  struct strobe_args *strobe = (struct strobe_args *)&arg->data.bytes;

  if (arg->next_time == 0) {
    arg->next_time = millis();
    strobe->next_sense = millis();
    strobe->on = false;
    strobe->default_period = 250;
    strobe->on_period = strobe->default_period;
    strobe->off_period = strobe->default_period;
  }

  if (millis() > arg->next_time) {
    if (strobe->on) {
      arg->next_time += strobe->off_period;
    } else {
      arg->next_time += strobe->on_period;
    }
    strobe->on = !(strobe->on);

    if (strobe->on) {
      setAllSquares(squares, size, arg->fgColor);
#ifdef ADDRESS_LIGHT_UNIT
#ifndef STROBE_PROGRAM
      sendHMTLValue(ADDRESS_LIGHT_UNIT, 3, 255);		    
#endif
#endif
    } else {
      setAllSquares(squares, size, arg->bgColor);
#ifdef ADDRESS_LIGHT_UNIT
#ifndef STROBE_PROGRAM
      sendHMTLValue(ADDRESS_LIGHT_UNIT, 3, 0);
#endif
#endif

    }
  }

  /* Handle the sensors with an independent timer */
  if (millis() > strobe->next_sense) {
    boolean update = false;
    strobe->next_sense += 10;

    if (CHECK_TOUCH_1()) {
      if (strobe->on_period > 0) {
	strobe->on_period--;
	strobe->off_period--;
	update = true;
      }
    }
    if (CHECK_TOUCH_2()) {
      if (strobe->on_period < 2000) {
	strobe->on_period++;
	strobe->off_period++;
	update = true;
      }
    }

#ifdef STROBE_PROGRAM
    if (update) {
      sendHMTLBlink(ADDRESS_LIGHT_UNIT, 0, 
		    strobe->on_period, arg->fgColor,
		    strobe->off_period, arg->bgColor);
    }
#endif    
  }
}
