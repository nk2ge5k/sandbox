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

// This uses the ‘haversine’ formula to calculate the great-circle distance
// between two points – that is, the shortest distance over the earth’s surface
// – giving an ‘as-the-crow-flies’ distance between the points (ignoring any
// hills they fly over, of course!).
//
// Haversine
// formula:	a = sin²(Δφ/2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ/2)
// c = 2 ⋅ atan2( √a, √(1−a) )
// d = R ⋅ c
// where	φ is latitude, λ is longitude, R is earth’s radius (mean radius
// = 6,371km); note that angles need to be in radians to pass to trig functions!
// JavaScript:
// const R = 6371e3; // metres
// const φ1 = lat1 * Math.PI/180; // φ, λ in radians
// const φ2 = lat2 * Math.PI/180;
// const Δφ = (lat2-lat1) * Math.PI/180;
// const Δλ = (lon2-lon1) * Math.PI/180;
//
// const a = Math.sin(Δφ/2) * Math.sin(Δφ/2) +
//           Math.cos(φ1) * Math.cos(φ2) *
//           Math.sin(Δλ/2) * Math.sin(Δλ/2);
// const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
//
// const d = R * c; // in metres
// Note in these scripts, I generally use lat/lon for lati­tude/longi­tude in
// degrees, and φ/λ for lati­tude/longi­tude in radians – having found that
// mixing degrees & radians is often the easiest route to head-scratching
// bugs...
//
// Historical aside: The height of tech­nology for navigator’s calculations used
// to be log tables. As there is no (real) log of a negative number, the
// ‘versine’ enabled them to keep trig func­tions in positive numbers. Also, the
// sin²(θ/2) form of the haversine avoided addition (which en­tailed an anti-log
// lookup, the addi­tion, and a log lookup). Printed tables for the
// haver­sine/in­verse-haver­sine (and its log­arithm, to aid multip­lica­tions)
// saved navi­gators from squaring sines, com­puting square roots, etc – arduous
// and error-prone activ­ities.
//
// The haversine formula1 ‘remains particularly well-conditioned for numerical
// computa­tion even at small distances’ – unlike calcula­tions based on the
// spherical law of cosines. The ‘(re)versed sine’ is 1−cosθ, and the
// ‘half-versed-sine’ is (1−cosθ)/2 or sin²(θ/2) as used above. Once widely used
// by navigators, it was described by Roger Sinnott in Sky & Telescope magazine
// in 1984 (“Virtues of the Haversine”): Sinnott explained that the angular
// separa­tion between Mizar and Alcor in Ursa Major – 0°11′49.69″ – could be
// accurately calculated in Basic on a TRS-80 using the haversine.
//
// For the curious, c is the angular distance in radians, and a is the square of
// half the chord length between the points.
//
// If atan2 is not available, c could be calculated from 2 ⋅ asin( min(1, √a) )
// (including protec­tion against rounding errors).
f64 haversineDistance(LngLat a, LngLat b) {
  f64 phi1 = degsToRads(a.lat);
  f64 phi2 = degsToRads(b.lat);
  f64 sin_dphi_2 = sin((phi2 - phi1) / 2.0f);
  f64 sin_dlam_2 = sin(degsToRads(b.lng - a.lng) / 2.0f);

  f64 hs = (sin_dphi_2 * sin_dphi_2) +
           (cos(phi1) * cos(phi2)) * (sin_dlam_2 * sin_dlam_2);

  return EARTH_RADIUS_M * (2 * atan2(sqrt(hs), sqrt(1 - hs)));
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

f64 projPseudoMercatorZoomForBBox(BBox *box, f64 size) {}

////////////////////////////////////////////////////////////////////////////////
/// TEST
////////////////////////////////////////////////////////////////////////////////

#if TEST_BUILD

#include <raymath.h>

internal bool f64equals(f64 x, f64 y) { return (fabs(x - y)) <= 0.00001f; }

internal void testMercatorProjection() {
  LngLat src = {
      .lng = -45.334071667255074L,
      .lat = 67.20373181860396L,
  };

  // Forward projection
  Vector2 vec = projMercator(src);
  Vector2 expect = {.x = -0.791229, .y = 1.601462};

  test_expect(f64equals(vec.x, expect.x) && f64equals(vec.y, expect.y),
              "Mercator projection (%f, %f) == (%f, %f)", //
              vec.x, vec.y,                               //
              expect.x, expect.y);

  // Inverse projection
  LngLat inv = projMercatorInverse(vec);
  test_expect(f64equals(inv.lng, src.lng) && f64equals(inv.lat, src.lat),
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

  test_expect(f64equals(vec.x, expect.x) && f64equals(vec.y, expect.y),
              "Pseudo-mercator projection (%f, %f) == (%f, %f)", //
              vec.x, vec.y,                                      //
              expect.x, expect.y);

  // Inverse projection
  LngLat inv = projPseudoMercatorInverse(vec);
  test_expect(f64equals(inv.lng, src.lng) && f64equals(inv.lat, src.lat),
              "Pseudo-mercator projection inverse (%f, %f) == (%f, %f)", //
              inv.lng, inv.lat,                                          //
              src.lng, src.lat);
}

internal void testHaversineDistance() {
  LngLat a = {
      .lng = -6.975393099350356,
      .lat = 48.52629507191503,
  };
  LngLat b = {
      .lng = 0.5861240498863651,
      .lat = 60.90859176834675,
  };

  f64 distance = haversineDistance(a, b);
  f64 expected = 1457707.921313;

  test_expect(
      f64equals(distance, expected),                                        //
      "Haversine distance between (%f, %f) and (%f, %f) = %f, expected %f", //
      a.lng, a.lat,                                                         //
      b.lng, b.lat,                                                         //
      distance, expected);
}

TEST(LNGLAT_H) {
  testMercatorProjection();
  testPseudoMercatorProjection();
  testHaversineDistance();
}

#endif
