#ifndef LNGLAT_H
#define LNGLAT_H


typedef struct LngLat {
  double lng;
  double lat;
} LngLat;

typedef struct BBox {
  struct LngLat sw; // aka minimum
  struct LngLat ne; // aka maximum
} BBox;

// Convert from decimal degrees to radians.
double degsToRads(double degrees);

// Convert from radians to decimal degrees.
double radsToDegs(double radians);

#endif
