#ifndef LNGLAT_H
#define LNGLAT_H

#include <raylib.h>

#include "types.h"

// LngLat represents position on the surface of the Earth
typedef struct LngLat {
  f64 lng;
  f64 lat;
} LngLat;

// lngLatZero returns zero initialized LngLat
#define lngLatZero()                                                           \
  (LngLat) { .lng = 0, .lat = 0 }

// BBox represents square on the surface of the earth
typedef struct BBox {
  LngLat sw; // minimum
  LngLat ne; // maximum
} BBox;

// lngLatZero returns zero initialized BBox
#define bboxZero()                                                             \
  (BBox) { .sw = lngLatZero(), .ne = lngLatZero() }
// Convert from decimal degrees to radians.
f64 degsToRads(f64 degrees);

// Convert from radians to decimal degrees.
f64 radsToDegs(f64 radians);

// bboxFromLngLat returns bounding box created form a single point.
BBox bboxFromLngLat(LngLat lnglat);

// bboxExpand expands rectangle to include given point.
BBox bboxExpandLngLat(BBox bbox, LngLat lnglat);

// bboxCenter returns center of the bounding box
LngLat bboxCenter(BBox bbox);

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

// projPseudoMercatorInverse returns geographical coordinates for the
// screen position.
// @slow: in most cases it will be slower to calculate zoom factor for
// the scale because it has been calculated already. Basically it is just
// a helper function.
LngLat projPseudoMercatorZoomedInverse(Vector2 vec, f64 zoom);

// projPseudoMercatorZoom returns scaling factor for given zoom level
f64 projPseudoMercatorZoom(f64 zoom);

// projPseudoMercatorSize returns size of the map, in pixels, for the given
// zoom level.
// NOTE(nk2ge5k): for the pseudo mercator projection map is a square, therefore
// i deiced to return size as single value.
f64 projPseudoMercatorSize(f64 zoom);

// projPseudoMercatorZoomForSize returns zoom value for the given map size in
// pixels.
f64 projPseudoMercatorZoomForSize(f64 size);

#if TEST_BUILD
#include "testhelp.h"
TEST(LNGLAT_H);
#endif

#endif
