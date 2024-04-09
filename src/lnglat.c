#include "lnglat.h"

#include <math.h>

#include "constans.h"
#include "debug.h"

f64 degsToRads(f64 degrees) { return degrees * M_PI_180; }

f64 radsToDegs(f64 radians) { return radians * M_180_PI; }

BBox bboxFromLngLat(LngLat lnglat) {
  BBox result = {
      .sw = lnglat,
      .ne = lnglat,
  };
  return result;
}

BBox bboxExpandLngLat(BBox bbox, LngLat lnglat) {
  BBox result = {
      .sw =
          {
              .lng = min_value(bbox.sw.lng, lnglat.lng),
              .lat = min_value(bbox.sw.lat, lnglat.lat),
          },
      .ne =
          {
              .lng = max_value(bbox.ne.lng, lnglat.lng),
              .lat = max_value(bbox.ne.lat, lnglat.lat),
          },
  };
  return result;
}

LngLat bboxCenter(BBox bbox) {
  LngLat result = {
      .lng = (bbox.sw.lng + bbox.ne.lng) / 2.0L,
      .lat = (bbox.sw.lat + bbox.ne.lat) / 2.0L,
  };
  return result;
}

f64 haversineDistance(LngLat a, LngLat b) {
  assertf(0, "haversineDistance is not implemented yet");
  return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
/// MERCATOR
////////////////////////////////////////////////////////////////////////////////

Vector2 projMercator(LngLat lnglat) {
  f64 lng_rads = degsToRads(lnglat.lng);
  f64 lat_rads = degsToRads(lnglat.lat);

  return (Vector2){
      .x = lng_rads,
      .y = log(tan(M_PI_4 + lat_rads / 2)),
  };
}

LngLat projMercatorInverse(Vector2 vec) {
  return (LngLat){
      .lng = radsToDegs(vec.x),
      .lat = radsToDegs(M_PI_2 - 2 * atan(pow(M_E, -vec.y))),
  };
}

////////////////////////////////////////////////////////////////////////////////
/// PSEUDO MERCATOR
////////////////////////////////////////////////////////////////////////////////

Vector2 projPseudoMercator(LngLat lnglat) {
  Vector2 vec = projMercator(lnglat);

  return (Vector2){
      .x = vec.x + M_PI,
      .y = M_PI - vec.y,
  };
}

LngLat projPseudoMercatorInverse(Vector2 vec) {
  LngLat lnglat = {
      .lng = radsToDegs(vec.x - M_PI),
      .lat = radsToDegs(M_PI_2 - 2 * atan(pow(M_E, vec.y - M_PI))),
  };
  return lnglat;
}

LngLat projPseudoMercatorZoomedInverse(Vector2 vec, f64 zoom) {
  f64 scale = projPseudoMercatorZoom(zoom);
  return projPseudoMercatorInverse((Vector2){
      .x = vec.x / scale,
      .y = vec.y / scale,
  });
}

f64 projPseudoMercatorZoom(f64 zoom) { return M_1_TAU * pow(2, zoom); }

f64 projPseudoMercatorSize(f64 zoom) { return pow(2.0L, zoom) - 1; }

f64 projPseudoMercatorZoomForSize(f64 size) { return log(size + 1) / M_LN2; }

f64 projPseudoMercatorZoomForBBox(BBox *box, f64 size) {
  f64 width = degsToRads(box->ne.lng - box->sw.lng);
  f64 height = degsToRads(box->ne.lat - box->sw.lat);
  f64 d = max_value(width, height);
  return log(size +1) / logf(d);
}

////////////////////////////////////////////////////////////////////////////////
/// TEST
////////////////////////////////////////////////////////////////////////////////

#if TEST_BUILD

#include <raymath.h>

internal void testMercatorProjection() {
  LngLat src = {
      .lng = -45.334071667255074L,
      .lat = 67.20373181860396L,
  };

  // Forward projection
  Vector2 vec = projMercator(src);
  Vector2 expect = {.x = -0.791229, .y = 1.601462};

  test_expect(FloatEquals(vec.x, expect.x) && FloatEquals(vec.y, expect.y),
              "Mercator projection (%f, %f) == (%f, %f)", //
              vec.x, vec.y,                               //
              expect.x, expect.y);

  // Inverse projection
  LngLat inv = projMercatorInverse(vec);
  test_expect(FloatEquals(inv.lng, src.lng) && FloatEquals(inv.lat, src.lat),
              "Mercator projection inverse (%f, %f) == (%f, %f)", //
              inv.lng, inv.lat,                                   //
              src.lng, src.lat);
}

internal void testPseudoMercatorProjection() {
  LngLat src = {
      .lng = -45.334071667255074L,
      .lat = 67.20373181860396L,
  };

  // Forward projection
  Vector2 vec = projPseudoMercator(src);
  Vector2 expect = {.x = 2.350364, .y = 1.540130};

  test_expect(FloatEquals(vec.x, expect.x) && FloatEquals(vec.y, expect.y),
              "Pseudo-mercator projection (%f, %f) == (%f, %f)", //
              vec.x, vec.y,                                      //
              expect.x, expect.y);

  // Inverse projection
  LngLat inv = projPseudoMercatorInverse(vec);
  test_expect(FloatEquals(inv.lng, src.lng) && FloatEquals(inv.lat, src.lat),
              "Pseudo-mercator projection inverse (%f, %f) == (%f, %f)", //
              inv.lng, inv.lat,                                          //
              src.lng, src.lat);
}


TEST(LNGLAT_H) {
  testMercatorProjection();
  testPseudoMercatorProjection();
}

#endif
