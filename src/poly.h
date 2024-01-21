#ifndef POLY_H
#include <stddef.h>
#include <stdio.h>

#include <raylib.h>

// Polygon
typedef struct Polygon {
  // Polygon vertices
  Vector2 *vertices;
  size_t vertices_size;
  // Indices of for vertices of triangles [A₁, B₁, C₁, A₂, B₂, C₂, ...]
  size_t *triangles;
  size_t triangles_size;
} Polygon;

// createPolygon allocates new polygon and fills is with triangles.
// NOTE(nk2ge5k): If polygon is not closed NULL will be returned.
// NOTE(nk2ge5k): Veritices are reused, not copied.
Polygon createPolygon(Vector2 *vertices, size_t size);

// freePolygon frees polygon
void freePolygon(Polygon *polygon);

// drawPolygon draws polygon and fills it with color by spliting polygon
// to the multiple triangles.
void drawPolygon(const Polygon *polygon, Vector2 skew, float scale,
                 Color color);

// drawPolygonLines draws polygon outline
void drawPolygonLines(const Polygon *polygon, Vector2 skew, float scale,
                      Color color);

#endif
