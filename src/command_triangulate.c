#include "command_triangulate.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>

#include "debug.h"
#include "poly.h"
#include "ui/text.h"

// Maximum number of points in ring
#define MAX_POINTS 128
// Ring points
global Vector2 ring[MAX_POINTS];
// Current ring length
global size_t ring_length = 0;

global bool triangulating;

// isClosed checks if ring is closed - first point is the same as last one.
internal bool isClosed() {
  if (ring_length < 3) {
    return false;
  }
  return ring[0].x == ring[ring_length - 1].x &&
         ring[0].y == ring[ring_length - 1].y;
}

// isVerticiesCloseBy checks if to verticies are close by so we could
// assume that it is the same point, logically.
internal bool isVerticiesCloseBy(Vector2 a, Vector2 b) {
  return fabs(a.x - b.x) < 10 && fabs(a.y - b.y) < 10;
}

internal void addPoint(Vector2 point) {
  if (ring_length >= MAX_POINTS || isClosed()) {
    // Reset ring if we reached max length.
    ring_length = 0;
  }

  // if (ring_length >= 2 && isVerticiesCloseBy(point, ring[0])) {
  //   ring[ring_length] = ring[0];
  //   ring_length++;
  //
  //   return;
  // }

  // Searching for point that close by, if such point exists then assing
  // same coordinates to the current point.
  // This helps with closing polygon as well as with simulating broken
  // polygon.
  for (size_t i = 0; i < ring_length; i++) {
    if (isVerticiesCloseBy(point, ring[i])) {
      point = ring[i];
      break;
    }
  }

  ring[ring_length] = point;
  ring_length++;
}

internal void frame() {
  ClearBackground(RAYWHITE);

  drawTextf(10,    // x
            10,    // y
            20,    // font size
            BLACK, // color
            "N: %llu", ring_length);

  if (isClosed()) {
    Polygon polygon = createPolygon(ring, NULL, ring_length - 1);
    drawPolygon(&polygon, (Vector2){.x = 0, .y = 0}, 1, BLUE);
    freePolygon(&polygon);
  }

  for (size_t i = 0; i < ring_length; i++) {
    if (i > 0) {
      DrawLineEx(ring[i - 1], ring[i], 2, BLACK);
    }
    DrawCircleV(ring[i], 4, BLACK);
  }
}

i32 commandTriangulate(i32 argc, char **argv) {
  const i32 screen_width = 800;
  const i32 screen_height = 600;

  InitWindow(screen_width, screen_height, "Drawing polygons");
  SetTargetFPS(30);

  while (!WindowShouldClose()) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      addPoint(GetMousePosition());
    }

    BeginDrawing();
    frame();
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
