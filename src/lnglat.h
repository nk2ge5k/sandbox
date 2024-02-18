#ifndef LNGLAT_H
#define LNGLAT_H

#include <raylib.h>

#include "types.h"

// LngLat represents position on the surface of the Earth
typedef struct LngLat {
  f64 lng;
  f64 lat;
} LngLat;

// BBox represents square on the surface of the earth
typedef struct BBox {
  LngLat sw; // minimum
  LngLat ne; // maximum
} BBox;

// lngLatZero returns zero initialized LngLat
#define lngLatZero()                                                           \
  (LngLat) { .lng = 0, .lat = 0 }

// Convert from decimal degrees to radians.
f64 degsToRads(f64 degrees);

// Convert from radians to decimal degrees.
f64 radsToDegs(f64 radians);

// Calculates haversine distance between two points on the sphere
// TODO(nk2ge5k): not implemented yet
f64 haversineDistance(LngLat a, LngLat b);

// projMercator returnts screen coordinates of the geo point using
// mertacor projection system.
// https://en.wikipedia.org/wiki/Mercator_projection
Vector2 projMercator(LngLat lnglat);

// projMercatorInverse returns geographical coordinates for the
// screen position.
LngLat projMercatorInverse(Vector2 vec);

// projPseudoMercator returnts screen coordinates of the geo point using
// pseudo/web mertacor projection system.
// https://en.wikipedia.org/wiki/Web_Mercator_projection
Vector2 projPseudoMercator(LngLat lnglat);

// projPseudoMercatorInverse returns geographical coordinates for the
// screen position.
LngLat projPseudoMercatorInverse(Vector2 vec);

// projPesudoMercatorZoom returns scaling factor for given zoom level
f64 projPesudoMercatorZoom(f64 zoom);

#if TEST_BUILD
#include "testhelp.h"
TEST(LNGLAT_H);
#endif

#endif
