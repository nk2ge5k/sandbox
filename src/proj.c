#include "proj.h"

#include <math.h>

#include "lnglat.h"

// Vector2 proj_mertacor(double lng, double lat) {
//   // TODO(nk2ge5k): read about width. Is it a pixel width? What else can it be?
//   int width = GetScreenWidth();
//   int height = GetScreenHeight();
//
//   // NOTE(nk2ge5k): this also works kind a...
//   // float x_radius =
//   //     (float)width / (2.0f * degs_to_rads(bounds->ne.lng - bounds->sw.lng));
//   // float y_radius =
//   //     (float)height / (2.0f * degs_to_rads(bounds->ne.lat - bounds->sw.lat));
//
//   float radius = (float)width / (2.0f * M_PI);
//   float lng_rads = degs_to_rads(lng);
//   float lat_rads = degs_to_rads(lat);
//   float y_from_equator = radius * log(tan(M_PI_4 + lat_rads / 2));
//
//   return (Vector2){
//       .x = (float)width / 2.0f + lng_rads * radius,
//       .y = (float)height / 2.0f - y_from_equator,
//   };
// }

// project_web_mertacor returnts screen coordinates of the geo point using
// web mertacor projection system.
// https://en.wikipedia.org/wiki/Web_Mercator_projection
Vector2 projectWebMertacor(double lng, double lat /* , float zoom */) {
  // const float ratio = (1.0f / M_2_PI) * pow(2, zoom);

  const float lng_rads = degs_to_rads(lng);
  const float lat_rads = degs_to_rads(lat);

  return (Vector2){
      .x = (lng_rads + M_PI),
      .y = (M_PI - log(tan(M_PI_4 + lat_rads / 2))),
  };
}
