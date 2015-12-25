Object Lights
=============

This code powers Adam Phelps's geometric lights

It is coded for the Arduino IDE and relies on several of my other libraries
* https://github.com/aphelps/ArduinoLibs
* https://github.com/aphelps/HTML

This code is released under the Creative Common's Non-Commercial license:
  http://creativecommons.org/licenses/by-nc/3.0/


TriangleLights Configuration
============================

Configuration is performed using the TriangleConfigure sketch

Notes:
* Using the configuration sketch, the triangle's LEDs should be R - G - B going clockwise, within a triangle these are LEDs 0 (R), 1 (G), and 2 (B)
* The edges are numbered to the right of the pixel with the same number.  Thus the R <-> G edge is 0, G <-> B is 1, and B <-> R is 2
* It is generally best to work through the triangles in the order the LEDs are installed, so LED 0 is the red LED of traingle 0, LED 3 is the red LED of triangle 1, etc

1) Initialize by sequencing the faces
’T 0 0’
* This assumes that the pixels are used sequentially.  If there is a gap or this doesn’t work for some other reason then use c/p/n and set the individual pixels.

2) Iterate over the faces correcting out of order LEDs
‘f 0’ to go to first face
‘R’ to reverse the G & B LEDs if needed
’N’ to advance the next face

2.1) Save the current state
‘write’

3) Iterate over the faces setting neighbors

‘F 0’ to go to the first face and color its neighbors

* Find the neighboring faces by iterating through the faces (’N’/‘P’/setting face)
* Set the edges once you have it:
‘E 0 6 1 19’

3.1) Save regularly:
‘write’

4) Verify

‘v’
