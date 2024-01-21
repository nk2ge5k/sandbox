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
#include <string.h>

#include <raylib.h>
#include <raymath.h>

#include "debug.h"
#include "lnglat.h"
#include "types.h"

////////////////////////////////////////////////////////////////////////////////
/// LINE
////////////////////////////////////////////////////////////////////////////////

// Maximum number of points in ring
#define MAX_POINTS 128

#define SQR(a) (a * a)

typedef struct Line {
  // Line points
  Vector2 vertices[MAX_POINTS];
  // Line length
  size_t length;
} Line;

// lineAppendVertex appends vertex to the end of the line returns false if
// line is null or line reached maximum size.
internal bool lineAppendVertex(Line *dst, Vector2 vec) {
  if (NULL == dst) {
    return false;
  }

  if (dst->length >= MAX_POINTS) {
    fprintf(stderr, "Cannot add more points - line reached maximum size\n");
    return false;
  }

  dst->vertices[dst->length] = vec;
  dst->length++;

  return true;
}

// lineVerticiesDraw draws every vertex of the line on the screen
internal void lineVerticiesDraw(const Line *line, int radius, Color color) {
  for (size_t i = 0; i < line->length; i++) {
    DrawCircleV(line->vertices[i], radius, color);
  }
}

// lineDraw draws the line on the screen
internal void lineDraw(const Line *line, float thick, Color color) {
  for (size_t i = 1; i < line->length; i++) {
    DrawLineEx(line->vertices[i - 1], // start
               line->vertices[i],     // end
               thick,                 // thickness of the line
               color);
  }
}

internal void lineClear(Line *line) { line->length = 0; }

////////////////////////////////////////////////////////////////////////////////
/// MultiLine
////////////////////////////////////////////////////////////////////////////////

typedef struct MultiLine {
  // Vertices of the lines
  Vector2 vertices[MAX_POINTS];
  // Lengths for the each line
  size_t lengths[MAX_POINTS];
  // Number of lines
  size_t length;
} MultiLine;

internal bool multilineAddLineVec(MultiLine *mline, const Vector2 *vertices,
                                  size_t length) {
  if (length < 2) {
    fprintf(stderr, "Line with length %ld is too small\n", length);
    return false;
  }

  size_t offset = 0;
  for (size_t i = 0; i < mline->length; i++) {
    offset += mline->lengths[i];
  }

  if (offset + length > MAX_POINTS) {
    fprintf(stderr, "Cannot add more lines - multiline reached maximum size\n");
    return false;
  }

  memcpy(&mline->vertices[offset], vertices, sizeof(Vector2) * length);
  mline->lengths[mline->length] = length;
  mline->length++;

  return true;
}

internal bool multilineAddLine(MultiLine *mline, Line *line) {
  return multilineAddLineVec(mline, line->vertices, line->length);
}

internal void multilineDraw(const MultiLine *mline, float thick, Color color) {
  size_t i, j;
  size_t offset = 0;
  for (i = 0; i < mline->length; i++) {
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

internal void multilineClear(MultiLine *mline) { mline->length = 0; }

////////////////////////////////////////////////////////////////////////////////
/// UTILS
////////////////////////////////////////////////////////////////////////////////

void drawTextf(int x, int y, int font_size, Color color, const char *format,
               ...) {
  static char buf[1024];
  {
    va_list argptr;
    va_start(argptr, format);
    vsprintf(buf, format, argptr);
    va_end(argptr);
  }

  DrawText(buf, x, y, font_size, color);
}

////////////////////////////////////////////////////////////////////////////////
/// BUTTON
////////////////////////////////////////////////////////////////////////////////

typedef struct Button {
  char *text;
  int font_size;
  float padding;
  Rectangle rect;
} Button;

// buttonFromText creates new button from the text.
internal Button buttonFromText(char *text, int font_size, float x, float y,
                               float padding) {
  int spacing = font_size / 10;

  Vector2 size = MeasureTextEx(GetFontDefault(), text, font_size, spacing);

  Button button = {
      .text = text,
      .font_size = font_size,
      .padding = padding,
      .rect =
          {
              .x = x,
              .y = y,
              .width = size.x + (padding * 2),
              .height = size.y + (padding * 2),
          },
  };

  return button;
}

internal void buttonDraw(Button *button, Color bg, Color stroke, Color font_color) {
  int spacing = button->font_size / 10;

  DrawRectangleRec(button->rect, bg);
  DrawTextEx(GetFontDefault(), // font
             button->text,     // text
             (Vector2){
                 .x = button->rect.x + button->padding,
                 .y = button->rect.y + button->padding,
             },                 // position
             button->font_size, // font size
             spacing,           // spacing
             font_color         // color
  );

  DrawRectangleLinesEx(button->rect, 2, stroke);
}

internal bool buttonCheckCollisionVec(Button *button, Vector2 vec) {
  return CheckCollisionPointRec(vec, button->rect);
}

////////////////////////////////////////////////////////////////////////////////
/// Ramer–Douglas–Peucker algorithm
////////////////////////////////////////////////////////////////////////////////

typedef struct Indicies {
  size_t v[MAX_POINTS];
  size_t length;
} Indicies;

internal void indiciesAppend(Indicies *dst, size_t index) {
  dst->v[dst->length] = index;
  dst->length++;
}

internal void indiciesClear(Indicies *dst) { dst->length = 0; }

internal float perpendicularDistance(Vector2 p, Vector2 start, Vector2 end) {
  float a = p.x - start.x;

  float b = p.y - start.y;
  float c = end.x - start.x;
  float d = end.y - start.y;
  if (c == 0 && d == 0) {
    return 0;
  }
  return fabs((a * d - c * b) / (c * c + d * d));
}

// Forward declaration of the function since it is a recursive function.
internal void ramerDouglasPeuker(Indicies *dst, Vector2 *vertices, size_t start,
                                 size_t end, float epsilon);

internal void ramerDouglasPeuker(Indicies *dst, Vector2 *vertices, size_t start,
                                 size_t end, float epsilon) {
  float max_distance = 0; // maximum distance
  size_t index;           // index of the max distance point

  for (size_t i = start + 1; i < end; i++) {
    float distance =
        perpendicularDistance(vertices[i], vertices[start], vertices[end]);
    assert(distance >= 0.0f);
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
  float angle;
  for (size_t i = 1; i < line->length; i++) {
    angle =
        radsToDegs(Vector2LineAngle(line->vertices[i - 1], line->vertices[i]));
    printf("Segment #%ld angle: %f\n", i, angle);
    angle = radsToDegs(Vector2LineAngle(Vector2Normalize(line->vertices[i - 1]),
                                        Vector2Normalize(line->vertices[i])));
    printf("Segment #%ld angle(normalized): %f\n", i, angle);
  }
}

internal Line approximateStraight(const Vector2 *vertices, size_t length) {
  Line line = {};
  size_t i;
  float a, b;
  float sum_x = 0, sum_y = 0;
  float sum_sqr_x = 0, sum_xy = 0;

  if (length < 2) {
    return line;
  }

  for (i = 0; i < length; i++) {
    sum_x += vertices[i].x;
    sum_y += vertices[i].y;
    sum_xy += (vertices[i].x * vertices[i].y);
    sum_sqr_x += (vertices[i].x * vertices[i].x);
  }

  b = ((float)length * sum_xy - sum_x * sum_y) /
      ((float)length * sum_sqr_x - sum_x * sum_x);
  a = (sum_y - b * sum_x) / (float)length;

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
  float target_angle;   // "golden" angle
  float current_angle;  // angle of the current line (two points)
  float diff;           // difference between current angle and golden one
  Line approx;          // approximated line for the segment
  MultiLine mline = {}; // resulting multiline

  if (line->length < 2) {
    return mline;
  }

  // Lets get "golden" angle from the first segment
  target_angle = radsToDegs(Vector2LineAngle(line->vertices[0], // start
                                             line->vertices[1]  // end
                                             ));

  for (size_t i = 2; i < line->length; i++) {
    current_angle = radsToDegs(Vector2LineAngle(line->vertices[i - 1], // start
                                                line->vertices[i]      // end
                                                ));

    diff = fabs(target_angle - current_angle);
    if (diff >= 85.0f) {
      printf("Breaking approximation on the segment #%ld, angle %.2f, target "
             "%.2f, difference %.2f\n",
             i, current_angle, target_angle, diff);
      approx = approximateStraight(&line->vertices[start], i - start);
      printf("Aproximated line with length %ld\n", approx.length);
      printf("Angle from the end of approximation %.2f\n",
             Vector2LineAngle(approx.vertices[approx.length - 1],
                              line->vertices[i]));
      multilineAddLine(&mline, &approx);

      target_angle = current_angle;
      start = i - 1;
    }
  }

  if (start < line->length - 1) {
    approx = approximateStraight(&line->vertices[start], line->length - start);
    printf("Aproximated line with length %ld\n", approx.length);
    multilineAddLine(&mline, &approx);
  }

  printf("Aproximated %ld lines\n", mline.length);

  return mline;
}

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

global float epsilon = 0.0f;

internal void frame() {
  ClearBackground(RAYWHITE);

  Vector2 cursor = GetMousePosition();
  bool is_over_run = buttonCheckCollisionVec(&button_run, cursor);
  bool is_over_clear = buttonCheckCollisionVec(&button_clear, cursor);

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
                         0,                     // start
                         drawn_line.length - 1, // end
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

  lineDraw(&drawn_line, 3, simple_line.length == 0 ? BLACK : LIGHTGRAY);
  lineVerticiesDraw(&drawn_line, 5,
                    simple_line.length == 0 ? BLACK : LIGHTGRAY);

  // Drawing simplified line
  for (size_t i = 1; i < simple_line.length; i++) {
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

int commandFixLine(int argc, char **argv) {
  const int screen_width = 800;
  const int screen_height = 600;

  InitWindow(screen_width, screen_height, "Drawing lines");
  SetTargetFPS(30);

  button_run = buttonFromText("RUN", 30, 10, 10, 10);
  button_clear =
      buttonFromText("CLEAR",
                     30, // font size
                     button_run.rect.x + button_run.rect.width + 10, // x
                     10,                                             // y
                     10                                              // padding
      );

  while (!WindowShouldClose()) {
    BeginDrawing();
    frame();
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
