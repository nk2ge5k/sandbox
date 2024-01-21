#include "lnglat.h"

/** pi / 180 */
#define M_PI_180 0.0174532925199432957692369076848861271111L
/** 180 / pi */
#define M_180_PI 57.29577951308232087679815481410517033240547L

double degs_to_rads(double degrees) { return degrees * M_PI_180; }

double rads_to_degs(double radians) { return radians * M_180_PI; }
