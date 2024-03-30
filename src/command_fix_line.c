/**
 * =============================================================================
 * TODO
 * =============================================================================
 *   - Factor out ramerDouglasPeuker
 *   - See if ramerDouglasPeuker can be not a recursive function
 *   - There is a suggestion here https://karthaus.nl/rdp/
 *     to use "Shortest path" instead of perpendicularDistance but as far as
 *     I can understand perpendicular is a shortest distance. I need to
 *     understand the difference.
 *   - Think about fast path for the ramerDouglasPeuker function
 */
#include "command_fix_line.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>
#include <stb_ds.h>

#include "csv.h"
#include "debug.h"
#include "lnglat.h"
#include "str.h"
#include "ui/button.h"
#include "ui/text.h"

////////////////////////////////////////////////////////////////////////////////
/// LINE
////////////////////////////////////////////////////////////////////////////////

// Maximum number of points in ring
#define MAX_POINTS 1024

typedef struct Line {
  // Line points
  Vector2 *vertices;
} Line;

typedef struct Geometry {
  // Geomtry vertices projected from geo coodinates
  Vector2 *vertices;
  // Geometry bounding box
  // @question: Can I return calculated values such as scale and offset?
  BBox bbox;
} Geometry;

// lineAppendVertex appends vertex to the end of the line returns false if
// line is null or line reached maximum size.
internal bool lineAppendVertex(Line *dst, Vector2 vec) {
  if (NULL == dst) {
    return false;
  }

  arrpush(dst->vertices, vec);

  return true;
}

// lineVerticiesDraw draws every vertex of the line on the screen
internal void lineVerticiesDraw(Vector2 *vertices, i32 radius, Color color,
                                Vector2 offset, f64 scale) {
  for (int i = 0; i < arrlen(vertices); i++) {
    DrawCircleV(Vector2Add(Vector2Scale(vertices[i], scale), offset), radius,
                color);
  }
}

// lineDraw draws the line on the screen
internal void lineDraw(Vector2 *vertices, f32 thick, Color color,
                       Vector2 offset, f64 scale) {
  for (i32 i = 1; i < arrlen(vertices); i++) {
    DrawLineEx(
        Vector2Add(Vector2Scale(vertices[i - 1], scale), offset), // start
        Vector2Add(Vector2Scale(vertices[i], scale), offset),     // end
        thick, // thickness of the line
        color);
  }
}

internal void lineClear(Line *line) { arrsetlen(line->vertices, 0); }

////////////////////////////////////////////////////////////////////////////////
/// MultiLine
////////////////////////////////////////////////////////////////////////////////

typedef struct MultiLine {
  // Vertices of the lines
  Vector2 *vertices;
  // Lengths for the each line
  int *lengths;
} MultiLine;

internal bool multilineAddLineVec(MultiLine *mline, const Vector2 *vertices,
                                  size_t length) {
  if (length < 2) {
    fprintf(stderr, "Line with length %ld is too small\n", length);
    return false;
  }

  int offset = 0;
  for (int i = 0; i < arrlen(mline->lengths); i++) {
    offset += mline->lengths[i];
  }

  if (offset + length > MAX_POINTS) {
    fprintf(stderr, "Cannot add more lines - multiline reached maximum size\n");
    return false;
  }

  arrinsn(mline->vertices, offset, length);
  memcpy(&mline->vertices[offset], vertices, sizeof(Vector2) * length);
  arrpush(mline->lengths, length);

  return true;
}

internal bool multilineAddLine(MultiLine *mline, Line *line) {
  return multilineAddLineVec(mline, line->vertices, arrlen(line->vertices));
}

internal void multilineDraw(const MultiLine *mline, f32 thick, Color color) {
  i32 i, j;
  i32 offset = 0;
  for (i = 0; i < arrlen(mline->lengths); i++) {
    for (j = 1; j < mline->lengths[i]; j++) {
      DrawLineEx(mline->vertices[offset + j - 1], // start
                 mline->vertices[offset + j],     // end
                 thick,                           // thickness of the line
                 color);
    }
    DrawCircleV(mline->vertices[offset], 5, GREEN);
    DrawCircleV(mline->vertices[offset + mline->lengths[i] - 1], 5, BLUE);
    offset += mline->lengths[i];
  }
}

internal void multilineClear(MultiLine *mline) {
  arrsetlen(mline->vertices, 0);
  arrsetlen(mline->lengths, 0);
}

////////////////////////////////////////////////////////////////////////////////
/// Ramer–Douglas–Peucker algorithm
////////////////////////////////////////////////////////////////////////////////

typedef struct Indicies {
  size_t *v;
} Indicies;

internal void indiciesAppend(Indicies *dst, size_t index) {
  arrpush(dst->v, index);
}

internal void indiciesClear(Indicies *dst) { arrsetlen(dst->v, 0); }

internal f32 perpendicularDistance(Vector2 p, Vector2 start, Vector2 end) {
  f32 a = p.x - start.x;

  f32 b = p.y - start.y;
  f32 c = end.x - start.x;
  f32 d = end.y - start.y;
  if (c == 0 && d == 0) {
    return 0;
  }
  return fabs((a * d - c * b) / (c * c + d * d));
}

// Forward declaration of the function since it is a recursive function.
internal void ramerDouglasPeuker(Indicies *dst, Vector2 *vertices, size_t start,
                                 size_t end, f32 epsilon);

internal void ramerDouglasPeuker(Indicies *dst, Vector2 *vertices, size_t start,
                                 size_t end, f32 epsilon) {
  f32 max_distance = 0; // maximum distance
  size_t index;           // index of the max distance point

  for (size_t i = start + 1; i < end; i++) {
    f32 distance =
        perpendicularDistance(vertices[i], vertices[start], vertices[end]);
    assertf(distance >= 0.0f, "Invalid perpendicular distance %f < 0",
            distance);
    if (distance > max_distance) {
      index = i;
      max_distance = distance;
    }
  }

  if (max_distance <= epsilon) {
    // If maximal distance of the line is less then epsilon we basically
    // dealing with the straight line that contains more then 2 points or
    // we just have two points.
    indiciesAppend(dst, start);
    indiciesAppend(dst, end);
  } else {
    ramerDouglasPeuker(dst, vertices, start, index, epsilon);
    ramerDouglasPeuker(dst, vertices, index, end, epsilon);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// LINE APPROXIMATION
////////////////////////////////////////////////////////////////////////////////

internal void printAngles(const Line *line) {
  f32 angle;
  for (i32 i = 1; i < arrlen(line->vertices); i++) {
    angle = radsToDegs(Vector2LineAngle(line->vertices[i - 1], line->vertices[i]));
    printf("Segment #%d angle: %f\n", i, angle);
    angle = radsToDegs(Vector2LineAngle(Vector2Normalize(line->vertices[i - 1]),
                                        Vector2Normalize(line->vertices[i])));
    printf("Segment #%d angle(normalized): %f\n", i, angle);
  }
}

internal Line approximateStraight(const Vector2 *vertices, size_t length) {
  Line line = {};
  size_t i;
  f32 a, b;
  f32 sum_x = 0, sum_y = 0;
  f32 sum_sqr_x = 0, sum_xy = 0;

  if (length < 2) {
    return line;
  }

  for (i = 0; i < length; i++) {
    sum_x += vertices[i].x;
    sum_y += vertices[i].y;
    sum_xy += (vertices[i].x * vertices[i].y);
    sum_sqr_x += (vertices[i].x * vertices[i].x);
  }

  b = ((f32)length * sum_xy - sum_x * sum_y) /
      ((f32)length * sum_sqr_x - sum_x * sum_x);
  a = (sum_y - b * sum_x) / (f32)length;

  for (i = 0; i < length; i++) {
    lineAppendVertex(&line, //
                     (Vector2){
                         .x = vertices[i].x,
                         .y = a + b * vertices[i].x,
                     });
  }

  return line;
}

internal MultiLine approximateStraights(const Line *line) {
  size_t start = 0;     // start index of current segment
  f32 target_angle;   // "golden" angle
  f32 current_angle;  // angle of the current line (two points)
  f32 diff;           // difference between current angle and golden one
  Line approx;          // approximated line for the segment
  MultiLine mline = {}; // resulting multiline

  if (arrlen(line->vertices) < 2) {
    return mline;
  }

  // Lets get "golden" angle from the first segment
  target_angle = radsToDegs(Vector2LineAngle(line->vertices[0], // start
                                             line->vertices[1]  // end
                                             ));

  for (int i = 2; i < arrlen(line->vertices); i++) {
    current_angle = radsToDegs(Vector2LineAngle(line->vertices[i - 1], // start
                                                line->vertices[i]      // end
                                                ));

    diff = fabs(target_angle - current_angle);
    if (diff >= 85.0f) {
      printf("Breaking approximation on the segment #%d, angle %.2f, target "
             "%.2f, difference %.2f\n",
             i, current_angle, target_angle, diff);
      approx = approximateStraight(&line->vertices[start], i - start);

      printf("Aproximated line with length %ld\n", arrlen(approx.vertices));
      printf("Angle from the end of approximation %.2f\n",
             Vector2LineAngle(arrlast(approx.vertices), line->vertices[i]));
      multilineAddLine(&mline, &approx);

      target_angle = current_angle;
      start = i - 1;
    }
  }

  if ((int)start < arrlen(line->vertices) - 1) {
    approx = approximateStraight(&line->vertices[start],
                                 arrlen(line->vertices) - start);
    printf("Aproximated line with length %ld\n", arrlen(approx.vertices));
    multilineAddLine(&mline, &approx);
  }

  printf("Aproximated %ld lines\n", arrlen(mline.lengths));

  return mline;
}

// FIXME: allocate arrays for the global arrays

// global line that will store points added but the user
global Line drawn_line;
// Simplified line
global Indicies simple_line;
// Approximated line
global MultiLine approximation;

// Button that suppose to activate straightening algorithm
global Button button_run = {};
// Button that suppose to clear lines from the screen
global Button button_clear = {};

global f32 epsilon = 0.0f;

internal void assetFrame(Geometry *asset) {
  ClearBackground(RAYWHITE);

  Vector2 cursor = GetMousePosition();
  bool is_over_run = buttonIsCollidedV(&button_run, cursor);
  bool is_over_clear = buttonIsCollidedV(&button_clear, cursor);

  if (IsKeyDown(KEY_I)) {
    epsilon += 0.01;
  } else if (IsKeyDown(KEY_N)) {
    epsilon -= 0.01;
  }

  if (epsilon < 0.0f) {
    epsilon = 0.0f;
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (is_over_run) {
      indiciesClear(&simple_line);
      ramerDouglasPeuker(&simple_line, asset->vertices,
                         0,                       // start
                         arrlen(asset->vertices), // end
                         epsilon);

      // approximation = approximateStraights(&drawn_line);
    } else if (is_over_clear) {
      // Reset
      arrsetcap(asset->vertices, 0);
      arrfree(asset->vertices);
    }
  }

  buttonDraw(&button_run,
             is_over_run ? LIGHTGRAY : RAYWHITE, // background
             BLACK,                              // outline color
             BLACK                               // font color
  );

  buttonDraw(&button_clear,
             is_over_clear ? LIGHTGRAY : RAYWHITE, // background
             BLACK,                                // outline color
             BLACK                                 // font color
  );

  lineDraw(asset->vertices,                                // vertices
           3,                                              // thick
           arrlen(simple_line.v) == 0 ? BLACK : LIGHTGRAY, // color
           Vector2Zero(),                                  // offset
           1.0L                                            // scale
  );
  lineVerticiesDraw(asset->vertices,                                // vertices
                    5,                                              // radius
                    arrlen(simple_line.v) == 0 ? BLACK : LIGHTGRAY, // color
                    Vector2Zero(),                                  // offset
                    1.0L                                            // scale
  );

  // Drawing simplified line
  for (int i = 1; i < arrlen(simple_line.v); i++) {
    size_t a = simple_line.v[i - 1];
    size_t b = simple_line.v[i];

    DrawLineEx(drawn_line.vertices[a], // start
               drawn_line.vertices[b], // end
               3,                      // thickness of the line
               BLACK);
    DrawCircleV(drawn_line.vertices[a], 5, BLACK);
    DrawCircleV(drawn_line.vertices[b], 5, BLACK);
  }

  multilineDraw(&approximation, 3, RED);

  drawTextf(10,                                              // X
            button_run.rect.y + button_run.rect.height + 10, // Y
            20,                                              // font size
            BLACK,                                           // color
            "E: %.2f", epsilon);
}

internal void frame() {
  ClearBackground(RAYWHITE);

  Vector2 cursor = GetMousePosition();
  bool is_over_run = buttonIsCollidedV(&button_run, cursor);
  bool is_over_clear = buttonIsCollidedV(&button_clear, cursor);

  if (IsKeyDown(KEY_I)) {
    epsilon += 0.01;
  } else if (IsKeyDown(KEY_N)) {
    epsilon -= 0.01;
  }

  if (epsilon < 0.0f) {
    epsilon = 0.0f;
  }

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (is_over_run) {
      indiciesClear(&simple_line);
      ramerDouglasPeuker(&simple_line, drawn_line.vertices,
                         0,                               // start
                         arrlen(drawn_line.vertices) - 1, // end
                         epsilon);

      // approximation = approximateStraights(&drawn_line);
    } else if (is_over_clear) {
      lineClear(&drawn_line);
      indiciesClear(&simple_line);
      multilineClear(&approximation);
    } else {
      lineAppendVertex(&drawn_line, GetMousePosition());
    }
  }

  buttonDraw(&button_run,
             is_over_run ? LIGHTGRAY : RAYWHITE, // background
             BLACK,                              // outline color
             BLACK                               // font color
  );

  buttonDraw(&button_clear,
             is_over_clear ? LIGHTGRAY : RAYWHITE, // background
             BLACK,                                // outline color
             BLACK                                 // font color
  );

  lineDraw(drawn_line.vertices,                            // vertices
           3,                                              // radius
           arrlen(simple_line.v) == 0 ? BLACK : LIGHTGRAY, // color
           Vector2Zero(),                                  // offset
           1.0L                                            // scale
  );
  lineVerticiesDraw(drawn_line.vertices,                            // vertices
                    5,                                              // thick
                    arrlen(simple_line.v) == 0 ? BLACK : LIGHTGRAY, // color
                    Vector2Zero(),                                  // offset
                    1.0L                                            // scale
  );

  // Drawing simplified line
  for (int i = 1; i < arrlen(simple_line.v); i++) {
    size_t a = simple_line.v[i - 1];
    size_t b = simple_line.v[i];

    DrawLineEx(drawn_line.vertices[a], // start
               drawn_line.vertices[b], // end
               3,                      // thickness of the line
               BLACK);
    DrawCircleV(drawn_line.vertices[a], 5, BLACK);
    DrawCircleV(drawn_line.vertices[b], 5, BLACK);
  }

  multilineDraw(&approximation, 3, RED);

  drawTextf(10,                                              // X
            button_run.rect.y + button_run.rect.height + 10, // Y
            20,                                              // font size
            BLACK,                                           // color
            "E: %.2f", epsilon);
}

////////////////////////////////////////////////////////////////////////////////
/// FILE
////////////////////////////////////////////////////////////////////////////////

LngLat parseLngLat(String str) {
  char textbuf[128];
  // string format is (-5.727941989898682,41.27079391479492)
  str = stringSlice(str, 1, str.len - 2);

  i32 comma_pos = stringIndexOf(str, COMMA);
  assert(comma_pos > 0);

  stringCopy(textbuf, stringSlice(str, 0, comma_pos));
  f32 lng = atof(textbuf);

  stringCopy(textbuf, stringSlice(str, comma_pos + 1, str.len - comma_pos - 1));
  f32 lat = atof(textbuf);

  LngLat lnglat = {
      .lng = lng,
      .lat = lat,
  };

  return lnglat;
}

// loadFromFile attempts to load line from the file
Geometry loadFromFile(char *filename) {
  Geometry result = {
      .vertices = NULL,
      .bbox = bboxZero(),
  };

  FILE *file = fopen(filename, "r");
  if (NULL == file) {
    return result;
  }

  String content = stringMakeFromFile(file);
  fclose(file);

  // Get header line and count number of the columns in the header
  size_t ncolumns = csvCountColumns(csvGetLine(&content), COMMA);
  if (ncolumns == 0) {
    stringFree(content);
    return result;
  }

  // Counting lines
  // NOTE(nk2ge5k): I am doing this after counting columns because I want
  // to exclude header from the count.
  size_t nlines = csvCountLines(content);
  arrsetcap(result.vertices, nlines); // @leak

  String *line = malloc(sizeof(String) * ncolumns);
  // NOTE(nk2ge5k): Since I know which file I am parsing i wont attempt to
  // find where is my column.
  for (size_t i = 0; !stringIsEmpty(content); i++) {
    size_t nvalues = csvGetValues(line, ncolumns, csvGetLine(&content), COMMA);
    // NOTE(nk2ge5k): actually there is no actual need for this check
    // because every row must have the same number of columns.
    assertf(nvalues >= 5, "Invalid CSV row #%lu", i + 1);

    LngLat lnglat = parseLngLat(line[4]);
    if (arrlen(result.vertices) == 0) {
      result.bbox = bboxFromLngLat(lnglat);
    } else {
      result.bbox = bboxExpandLngLat(result.bbox, lnglat);
    }
    arrpush(result.vertices, projPseudoMercator(lnglat));
  }

cleanup:
  stringFree(content);
  free(line);

  return result;
}

i32 commandFixLine(i32 argc, char **argv) {
  Geometry asset = {};

  if (argc > 1) {
    // TODO(nk2ge5k): draw line to the screen.
    //  - I need to figure out how to calculate zoom and x, y offset for
    //    bounding box which is important for the drawing geo spatial data.
    asset = loadFromFile(argv[1]);
  }

  const i32 screen_width = 800;
  const i32 screen_height = 600;

  InitWindow(screen_width, screen_height, "Drawing lines");
  SetTargetFPS(30);

  button_run = buttonFromText("RUN", 30, (Vector2){.x = 10, .y = 10}, 10);
  button_clear =
      buttonFromText("CLEAR",
                     30, // font size
                     (Vector2){
                         .x = button_run.rect.x + button_run.rect.width + 10,
                         .y = 10,
                     },
                     10 // padding
      );

  while (!WindowShouldClose()) {
    BeginDrawing();
    if (arrlen(asset.vertices) == 0) {
      frame();
    } else {
      assetFrame(&asset);
    }
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
