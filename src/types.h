#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <inttypes.h>

// Keywords that allow for the simple separation of the static
#define global static
#define internal static

typedef int32_t i32;
typedef int64_t i64;

#define Fi32 "%" PRId32 ""
#define Fi64 "%" PRId64 ""

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define Fu32 "%" PRIu32 ""
#define Fu64 "%" PRIu64 ""

typedef float f32;
typedef double f64;

#define Ff32 "%f"
#define Ff64 "%lf"

// max_value returns maximum between two values.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define max_value(a, b) (((a) > (b)) ? (a) : (b))

// min_value returns maximum between two values.
// NOTE(nk2ge5k): use with caution - this macro must not be used with complex
// expressions as values, because it will force value recalculation.
#define min_value(a, b) (((a) < (b)) ? (a) : (b))


#endif
