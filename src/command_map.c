/**
 * =============================================================================
 * TODO
 * =============================================================================
 * - [poly.c] I need to find out what is wrong with the triangulation algorithm,
 *   something telling me that I did not understand how to filter vertices
 *   correctly by angle. Also I see that sometimes a lot of vertices are
 *   left in the linked list (more then 5, sometimes even more then 40) it
 *   tells me that algorithm cannot find ear after 3 full cycles through the
 *   polygon.
 *   I broke a part data/africa-europe.geobuf file to the different files,
 *   each one contains features for the one, single, country. I think it would
 *   help while debugging.
 */
#include "command_map.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>
#include <stb_ds.h>

#include "debug.h"
#include "file.h"
#include "lnglat.h"
#include "poly.h"
#include "proto/geobuf.pb-c.h"
#include "types.h"
#include "ui/text.h"

#define MAX_TEXT_LEN 128

////////////////////////////////////////////////////////////////////////////////
/// ASSETS
////////////////////////////////////////////////////////////////////////////////

internal Grass__V1__Geobuf *readGeobufFromFile(const char *filename) {
  debugf("reading geobuf file %s\n", filename);

  FILE *file = fopen(filename, "r");
  if (NULL == file) {
    errorf("Failed to open file %s: %s\n", filename, STD_ERROR);
    goto file_error;
  }

  int64_t file_size = fileSize(file);
  if (file_size < 0) {
    errorf("Failed to get file size of file %s\n", filename);
    goto file_error;
  }

  uint8_t *geobuf_data = malloc(sizeof(uint8_t) * file_size + 1);
  memset(geobuf_data, 0, file_size + 1);

  int64_t read = fileReadInto(geobuf_data, file_size, file);
  if (read != file_size) {
    errorf("Short read %lld != %lld\n", read, file_size);
    goto error;
  }

  Grass__V1__Geobuf *geobuf =
      grass__v1__geobuf__unpack(NULL, file_size, geobuf_data);
  if (NULL == geobuf) {
    errorf("Failed to unpack geobuf\n");
    goto error;
  }

  free(geobuf_data);
  return geobuf;
error: // Late errors after allocatiing geobuf_data struct
  free(geobuf_data);
file_error: // Early errors after opening file
  fclose(file);
  return NULL;
}

// readRing reads ring of given length from array of geobuf coordinates.
internal Vector2 *readRing(const int64_t *coords, const size_t length,
                           const float precision) {
  Vector2 *ring = malloc(sizeof(Vector2) * length);

  float lng, lat;
  size_t cur = 0;
  const size_t n_coords = length * 2;

  for (size_t i = 0; i < n_coords; i += 2, cur++) {
    lng = coords[i] / precision;
    lat = coords[i + 1] / precision;
    ring[cur] = projPseudoMercator((LngLat){
        .lng = lng,
        .lat = lat,
    });
  }

  return ring;
}

internal void appendPolygon(Polygon **dst, const float precision,
                            const Grass__V1__Geometry *geometry) {
  assert(geometry->n_lengths >= 2);
  // TODO(nk2ge5k): for now we do not handle hole just exteriors.
  size_t exterior_len = geometry->lengths[1];
  Vector2 *ring = readRing(geometry->coords, exterior_len, precision);
  debugf("Polygon #%ld\n", NULL == dst ? 0 : arrlen(*dst));
  arrput(*dst, createPolygon(ring, exterior_len));
}

internal void appendMultiPolygon(Polygon **dst, const float precision,
                                 const Grass__V1__Geometry *geometry) {
  assert(geometry->n_lengths >= 3);

  Vector2 *ring;
  uint32_t *lengths = geometry->lengths;
  int64_t *coords = geometry->coords;

  size_t n_polygons = *lengths;
  int n_rings;

  lengths++;

  for (size_t i = 0; i < n_polygons; i++) {
    n_rings = *lengths;
    lengths++; // move to the the ring length

    ring = readRing(coords, *lengths, precision);
    debugf("Polygon #%ld\n", NULL == dst ? 0 : arrlen(*dst));
    arrput(*dst, createPolygon(ring, *lengths));
    coords += (*lengths) * 2;

    n_rings--;
    lengths++;

    for (; n_rings > 0; n_rings--) {
      coords += (*lengths) * 2;
      lengths++;
    }
  }
}

internal char *stringProperty(const Grass__V1__Properties *props,
                              const int key) {
  if (props == NULL || key < 0) {
    return "<UNKNOWN>";
  }

  size_t i = 0;
  for (; i < props->n_keys; i++) {
    if (props->keys[i] == (uint32_t)key) {
      break;
    }
  }

  if (props->n_values <= i || props->values[i]->value_type_case !=
                                  GRASS__V1__VALUE__VALUE_TYPE_STRING_VALUE) {
    return "<UNKNOWN>";
  }

  return props->values[i]->string_value;
}

internal int propertyKey(const Grass__V1__Geobuf *geobuf, const char *name) {
  for (size_t i = 0; i < geobuf->n_keys; i++) {
    if (strcmp(name, geobuf->keys[i]) == 0) {
      return i;
    }
  }
  return -1;
}

internal void appendFeature(Polygon **dst, const float precision,
                            const int name_key,
                            const Grass__V1__Feature *feature) {
  debugf("Creating polygon for %s country\n",
         stringProperty(feature->properties, name_key));

  switch (feature->geometry->type) {
  case GRASS__V1__GEOMETRY_TYPE__GEOMETRY_TYPE_POLYGON:
    appendPolygon(dst, precision, feature->geometry);
    break;
  case GRASS__V1__GEOMETRY_TYPE__GEOMETRY_TYPE_MULTIPOLYGON:
    appendMultiPolygon(dst, precision, feature->geometry);
    break;
  default:
    break;
  }
}

internal void
appendFeatureCollection(Polygon **dst, const float precision,
                        const int name_key,
                        const Grass__V1__FeatureCollection *collection) {
  for (size_t i = 0; i < collection->n_features; i++) {
    appendFeature(dst, precision, name_key, collection->features[i]);
  }
}

internal Polygon *readPolygonsFromGeobuf(Grass__V1__Geobuf *geobuf) {
  const float precision = powf(10.0, geobuf->precision);
  const int name_key = propertyKey(geobuf, "name");
  Polygon *polygons = NULL;

  switch (geobuf->data_type_case) {
  case GRASS__V1__GEOBUF__DATA_TYPE_FEATURE_COLLECTION:
    appendFeatureCollection(&polygons, precision, name_key,
                            geobuf->feature_collection);
    break;
  case GRASS__V1__GEOBUF__DATA_TYPE_FEATURE:
    appendFeature(&polygons, precision, name_key, geobuf->feature);
    break;
  default:
    break;
  }

  return polygons;
}

int commandMap(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Error: missing geobuf file argument\n");
    fprintf(stderr, "  Usage: %s FILE\n", *argv);
    return 1;
  }

  const int screen_width = 800;
  const int screen_height = 600;

  Grass__V1__Geobuf *geobuf = readGeobufFromFile(argv[1]);
  if (geobuf == NULL) {
    fprintf(stderr, "Failed to read geobuf\n");
    return 1;
  }

  Polygon *polygons = readPolygonsFromGeobuf(geobuf);
  grass__v1__geobuf__free_unpacked(geobuf, NULL);

  size_t n_polygons = arrlen(polygons);
  if (n_polygons == 0) {
    fprintf(stderr, "no polygons found\n");
    return 1;
  }

  float zoom = 7;
  float scale = 1;

  Vector2 skew = {.x = 0, .y = 0};
  int i = 0;

  // Color color;
  debugf("Read %ld polygons\n", n_polygons);

  InitWindow(screen_width, screen_height, "Drawing polygons");
  SetTargetFPS(30);

  while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_J)) {
      zoom -= 1.0f / zoom;
    } else if (IsKeyDown(KEY_K)) {
      zoom += 1.0f / zoom;
    }

    if (IsKeyDown(KEY_UP)) {
      skew.y += 1.0f / zoom;
    } else if (IsKeyDown(KEY_DOWN)) {
      skew.y -= 1.0f / zoom;
    }

    if (IsKeyDown(KEY_LEFT)) {
      skew.x += 1.0f / zoom;
    } else if (IsKeyDown(KEY_RIGHT)) {
      skew.x -= 1.0f / zoom;
    }

    if (IsKeyPressed(KEY_N)) {
      i++;
      if ((size_t)i >= n_polygons) {
        i = 0;
      }
    } else if (IsKeyPressed(KEY_P)) {
      i--;
      if (i < 0) {
        i = n_polygons - 1;
      }
    }

    zoom = Clamp(zoom, 6.0f, 30.0f);
    scale = projPesudoMercatorZoom(zoom);
    Vector2 shift = Vector2Scale(skew, scale);

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawRectangleRec(
        (Rectangle){
            .x = shift.x,
            .y = shift.y,
            .width = pow(2.0L, zoom) - 1,
            .height = pow(2.0L, zoom) - 1,
        },
        DARKBLUE);

    for (size_t i = 0; i < n_polygons; i++) {
      drawPolygon(&polygons[i], shift, scale, DARKGREEN);
      drawPolygonLines(&polygons[i], shift, scale, BLACK);
    }

    drawTextf(10, 10, 20, BLACK, "ZOOM: %.2f", zoom);
    drawTextf(10, 30, 20, BLACK, "SKEW: (%.2f, %.2f)", skew.x, skew.y);
    drawTextf(10, 50, 20, BLACK, "SCAL: %.2f", scale);

    {
      int width = GetScreenWidth();
      int height = GetScreenHeight();
      DrawLine(0, height / 2, width, height / 2, BLACK);
      DrawLine(width / 2, 0, width / 2, height, BLACK);
    }
    EndDrawing();
  }

  for (size_t i = 0; i < n_polygons; i++) {
    free(polygons[i].vertices);
    freePolygon(&polygons[i]);
  }
  arrfree(polygons);

  return 0;
}
