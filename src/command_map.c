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

  i64 file_size = fileSize(file);
  if (file_size < 0) {
    errorf("Failed to get file size of file %s\n", filename);
    goto file_error;
  }

  u8 *geobuf_data = malloc(sizeof(u8) * file_size + 1);
  memset(geobuf_data, 0, file_size + 1);

  i64 read = fileReadInto(geobuf_data, file_size, file);
  if (read != file_size) {
    errorf("Short read " Fi64 " != " Fi64 "\n", read, file_size);
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
internal Vector2 *readRing(const i64 *coords, const size_t length,
                           const f32 precision, BBox *box) {

  BBox bbox;
  Vector2 *ring = malloc(sizeof(Vector2) * length); //@leak

  f32 lng, lat;
  size_t cur = 0;
  const size_t n_coords = length * 2;

  for (size_t i = 0; i < n_coords; i += 2, cur++) {
    LngLat lnglat = {
        .lng = coords[i] / precision,
        .lat = coords[i + 1] / precision,
    };

    if (i == 0) {
      bbox = bboxFromLngLat(lnglat);
    } else {
      bbox = bboxExpandLngLat(bbox, lnglat);
    }
    ring[cur] = projPseudoMercator(lnglat);
  }

  *box = bbox;

  return ring;
}

internal void appendPolygon(Polygon **dst, const f32 precision,
                            const Grass__V1__Geometry *geometry) {
  assert(geometry->n_lengths >= 2);
  BBox *bbox = malloc(sizeof(BBox)); //@leak
  // TODO(nk2ge5k): for now we do not handle hole just exteriors.
  size_t exterior_len = geometry->lengths[1];
  Vector2 *ring = readRing(geometry->coords, exterior_len, precision, bbox);
  debugf("Polygon #%ld\n", NULL == dst ? 0 : arrlen(*dst));
  arrput(*dst, createPolygon(ring, bbox, exterior_len));
}

internal void appendMultiPolygon(Polygon **dst, const f32 precision,
                                 const Grass__V1__Geometry *geometry) {
  assert(geometry->n_lengths >= 3);

  Vector2 *ring;
  u32 *lengths = geometry->lengths;
  i64 *coords = geometry->coords;

  size_t n_polygons = *lengths;
  i32 n_rings;

  lengths++;

  for (size_t i = 0; i < n_polygons; i++) {
    n_rings = *lengths;
    lengths++; // move to the the ring length

    BBox *bbox = NULL;
    ring = readRing(coords, *lengths, precision, bbox);
    debugf("Polygon #%ld\n", NULL == dst ? 0 : arrlen(*dst));
    arrput(*dst, createPolygon(ring, bbox, *lengths));
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
                              const i32 key) {
  if (props == NULL || key < 0) {
    return "<UNKNOWN>";
  }

  size_t i = 0;
  for (; i < props->n_keys; i++) {
    if (props->keys[i] == (u32)key) {
      break;
    }
  }

  if (props->n_values <= i || props->values[i]->value_type_case !=
                                  GRASS__V1__VALUE__VALUE_TYPE_STRING_VALUE) {
    return "<UNKNOWN>";
  }

  return props->values[i]->string_value;
}

internal i32 propertyKey(const Grass__V1__Geobuf *geobuf, const char *name) {
  for (size_t i = 0; i < geobuf->n_keys; i++) {
    if (strcmp(name, geobuf->keys[i]) == 0) {
      return i;
    }
  }
  return -1;
}

internal void appendFeature(Polygon **dst, const f32 precision,
                            const i32 name_key,
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
appendFeatureCollection(Polygon **dst, const f32 precision, const i32 name_key,
                        const Grass__V1__FeatureCollection *collection) {
  for (size_t i = 0; i < collection->n_features; i++) {
    appendFeature(dst, precision, name_key, collection->features[i]);
  }
}

internal Polygon *readPolygonsFromGeobuf(Grass__V1__Geobuf *geobuf) {
  const f32 precision = powf(10.0, geobuf->precision);
  const i32 name_key = propertyKey(geobuf, "name");
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

internal Vector2 mapOffset(f64 zoom) {

  f64 size = projPseudoMercatorSize(zoom);

  f64 maps_horizontal = (f64)GetScreenWidth() / size;
  f64 maps_vertical = (f64)GetScreenHeight() / size;

  Vector2 offset = {
      .x = size * (maps_horizontal / 2.0L),
      .y = size * (maps_vertical / 2.0L),
  };

  return offset;
}

internal LngLat lngLatForPosition(Vector2 pos, f64 zoom, Vector2 offset) {
  return projPseudoMercatorZoomedInverse(Vector2Subtract(pos, offset), zoom);
}

internal Vector2 calcOffsetFor(LngLat lnglat, f64 zoom) {
  Vector2 delta =
      Vector2Scale(projPseudoMercator(lnglat), projPseudoMercatorZoom(zoom));
  return Vector2Subtract(mapOffset(zoom), delta);
}

i32 commandMap(i32 argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Error: missing geobuf file argument\n");
    fprintf(stderr, "  Usage: %s FILE\n", *argv);
    return 1;
  }

  const i32 screen_width = 800;
  const i32 screen_height = 600;

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

  InitWindow(screen_width, screen_height, "Drawing polygons");
  SetTargetFPS(30);

  i32 i = 0;
  f32 zoom, pzoom;
  zoom = pzoom =
      projPseudoMercatorZoomForSize(min_value(screen_width, screen_height));
  Vector2 offset = calcOffsetFor(lngLatZero(), zoom);

  if (n_polygons == 1) {
    // TODO(nk2ge5k): fit polygon bounding box into view
    assertf(polygons[0].bbox != NULL, "Expected bounding box");
    zoom = pzoom = projPseudoMercatorZoomForBBox(
        polygons[0].bbox, min_value(screen_width, screen_height));
    Vector2 offset = calcOffsetFor(bboxCenter(*polygons[0].bbox), zoom);
  }

  while (!WindowShouldClose()) {

    if (IsKeyPressed(KEY_R)) {
      zoom = pzoom = 9.0f;
      offset = calcOffsetFor(lngLatZero(), zoom);
    }

    if (IsKeyDown(KEY_J)) {
      zoom -= 1.0f / zoom;
    } else if (IsKeyDown(KEY_K)) {
      zoom += 1.0f / zoom;
    }

    zoom = Clamp(zoom, 6.0f, 30.0f);
    f64 size = projPseudoMercatorSize(zoom);

    if (!FloatEquals(pzoom, zoom)) {
      // @slow: I do not like this double coversion - it fills like it can be
      // done more efficently, for example I could try and determine releative
      // position of the inside of the map, not sure that it would work.
      LngLat looked_at = lngLatForPosition(
          (Vector2){
              .x = GetScreenWidth() / 2.0f,
              .y = GetScreenHeight() / 2.0f,
          },
          pzoom, offset);
      offset = calcOffsetFor(looked_at, zoom);

      pzoom = zoom;
    }

    if (IsKeyDown(KEY_UP)) {
      offset.y += size / 180.0f;
    } else if (IsKeyDown(KEY_DOWN)) {
      offset.y -= size / 180.0f;
    }

    if (IsKeyDown(KEY_LEFT)) {
      offset.x += size / 180.0f;
    } else if (IsKeyDown(KEY_RIGHT)) {
      offset.x -= size / 180.0f;
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

    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawRectangleLinesEx(
        (Rectangle){
            .x = offset.x,
            .y = offset.y,
            .width = pow(2.0L, zoom) - 1,
            .height = pow(2.0L, zoom) - 1,
        },
        2.0f, BLACK);

    f64 scale = projPseudoMercatorZoom(zoom);
    for (size_t i = 0; i < n_polygons; i++) {
      // drawPolygon(&polygons[i], offset, scale, DARKGREEN);
      drawPolygonLines(&polygons[i], offset, scale, BLACK);
    }

    drawTextf(10, 10, 10, BLACK, "ZOOM: %.2f", zoom);
    drawTextf(10, 20, 10, BLACK, "OFFS: (%.2f, %.2f)", offset.x, offset.y);
    drawTextf(10, 30, 10, BLACK, "SCAL: %.2f", scale);
    DrawLine(0, screen_height / 2, screen_width, screen_height / 2, BLACK);
    DrawLine(screen_width / 2, 0, screen_width / 2, screen_height, BLACK);

    EndDrawing();
  }

  for (size_t i = 0; i < n_polygons; i++) {
    free(polygons[i].vertices);
    freePolygon(&polygons[i]);
  }
  arrfree(polygons);

  return 0;
}
