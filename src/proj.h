#ifndef PROJECTION_H
#define PROJECTION_H

#include <raylib.h>

// project_web_mertacor returnts screen coordinates of the geo point using
// mertator projection system that I found here:
// https://medium.com/@suverov.dmitriy/how-to-convert-latitude-and-longitude-coordinates-into-pixel-offsets-8461093cb9f5
// Vector2 proj_mertacor(double lng, double lat);

// project_web_mertacor returnts screen coordinates of the geo point using
// web mertacor projection system.
// https://en.wikipedia.org/wiki/Web_Mercator_projection
Vector2 projectWebMertacor(double lng, double lat /*, float zoom */);

#endif
