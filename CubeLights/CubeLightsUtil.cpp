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
	int red = squares[tri].leds[c].red();
 	int green = squares[tri].leds[c].green();
	int blue = squares[tri].leds[c].blue();

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

static unsigned long next_time = 0;

/* This iterates through the squares, lighting the ones with leds */
void squaresTestPattern(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  static int current = 0;

  if (init) {
    current = 0;
    next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

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
void squaresSetupPattern(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  static byte current = 0;
  static byte led = 0;

  if (init) {
    current = 0;
    led = 0;
    next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

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
void squaresRandomNeighbor(Square *squares, int size, int periodms,
			     boolean init, pattern_args_t *arg) {
  static Square *current = &squares[0];

  if (init) {
    current = &squares[0];
    next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

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

void squaresCyclePattern(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  static byte phase = 0;

  if (init) {
    phase = 0;
    next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

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


void squaresCirclePattern(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  static byte phase = 0;

  if (init) {
    phase = 0;
    next_time = millis();
    clearSquares(squares, size);
  }

  if (millis() > next_time) {
    next_time += periodms;

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

void squaresFadeCycle(Square *squares, int size, int periodms,
			  boolean init, pattern_args_t *arg) {
  static long phase_start = 0;
  long max_phase = 15 * 1000;
  static boolean fadeup = true;

  unsigned long now = millis();

  if (init) {
    phase_start = now;
    next_time = now;
    clearSquares(squares, size);
  }

  if (now > next_time) {
    long phase = now - phase_start;

    next_time += periodms;

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

void squaresAllOn(Square *squares, int size, int periodms,
		  boolean init, pattern_args_t *arg) {
  if (init) {
    next_time = millis();
  }

  if (millis() > next_time) {
    next_time += periodms;
    setAllSquares(squares, size, arg->fgColor);
  }
}

/*
 * This mode reacts to the capacitive touch sensors
 */
void squaresStaticNoise(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg) {
  if (init) {
    next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > next_time) {
    next_time += periodms;

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

void squaresSwitchRandom(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg) {
  if (init) {
    next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > next_time) {
    next_time += periodms;

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
void squaresCapResponse(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg) {
  
}


void squaresBarCircle(Square *squares, int size, int periodms,
		      boolean init, pattern_args_t *arg) {
  static Square *face = NULL;
  static byte current_bar = 0;

  if (init) {
    face = &squares[0];
    next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > next_time) {
    next_time += periodms;

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
void squaresCrawl(Square *squares, int size, int periodms,
		  boolean init, pattern_args_t *arg) {
  static Square *face = NULL;
  static byte led = 0;
  static byte color = 0;

  if (init) {
    face = &squares[0];
    led = 0;
    next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > next_time) {
    int timeincrement = periodms;
    if (touch_sensor.touched(CAP_SENSOR_1))
      timeincrement = timeincrement / 2;
    if (touch_sensor.touched(CAP_SENSOR_2))
      timeincrement = timeincrement / 4;
    next_time += timeincrement;

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

void squaresOrbits(Square *squares, int size, int periodms,
		  boolean init, pattern_args_t *arg) {
  static Square *face = NULL;
  static Square *prevface = NULL;
  static byte led = 0;
  //  static byte color = 0;

  if (init) {
    face = &squares[0];
    prevface = face->edges[Square::TOP];
    led = 8;
    next_time = millis();
    setAllSquares(squares, size, arg->bgColor);
  }

  if (millis() > next_time) {
    next_time += periodms;

    face->setColor(led, arg->bgColor);

    uint16_t next = face->ledAwayFrom(prevface, led);
    if (FACE_FROM_COMBO(next) == face->id) {
      led = LED_FROM_COMBO(next);
    } else {
      prevface = face;
      face = &squares[FACE_FROM_COMBO(next)];
    }
    face->setColor(led, arg->fgColor);

    DEBUG_VALUE(DEBUG_HIGH, "Orbit: ", face->id);
    DEBUG_VALUELN(DEBUG_HIGH, ":", led);
  }
}

/******************************************************************************
 * Followup functions
 */

unsigned long next_followup = 0;

/* 
 * Set the center LED of each square to the indicated color
 */
void squaresLightCenter(Square *squares, int size, int periodms,
			boolean init, pattern_args_t *arg) {
  if (init) {
    next_followup = millis();
  }

  if (millis() > next_followup) {
    next_followup += periodms;
    for (int square = 0; square < size; square++) {
      squares[square].setColor(4, arg->fgColor);
    }
  }
}

void squaresBlinkPattern(Square *squares, int size, int periodms,
			   boolean init, pattern_args_t *arg) {
  static boolean on = false;

  if (init) {
    next_followup = millis();
    on = false;
  }

  if (millis() > next_followup) {
    next_followup += periodms;
    on = !on;
  }

  uint32_t color;
  if (on) color = arg->fgColor;
  else color = arg->bgColor;

  byte face_mask = FACES_FROM_MASK(arg->data);
  for (byte face = 0; face < size; face++) {
    if (face_mask & (1 << face)) {
      for (byte led = 0; led < Square::NUM_LEDS; led++) {
	if (arg->data & (1 << led)) {
	  squares[face].setColor(led, color);
	}
      }
    }
  }
}
