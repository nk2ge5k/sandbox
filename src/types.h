#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Keywords that allow for the simple separation of the static
#define global static
#define internal static

typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

// max_value returns maximum between two values.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define max_value(a, b) (((a) > (b)) ? (a) : (b))

// min_value returns maximum between two values.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define min_value(a, b) (((a) < (b)) ? (a) : (b))

#endif
