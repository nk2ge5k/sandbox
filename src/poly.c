#include "poly.h"

#include <assert.h>
#include <raymath.h>
#include <stdlib.h>

#define DEBUG
#include "debug.h"

static void triangulate(Polygon *polgyon);

////////////////////////////////////////////////////////////////////////////////
/// API
////////////////////////////////////////////////////////////////////////////////

Polygon createPolygon(Vector2 *vertices, size_t size) {
  Polygon polygon = {
      .vertices = vertices,
      .vertices_size = size,
      .triangles = NULL,
      .triangles_size = 0,
  };

  triangulate(&polygon);

  return polygon;
}

void freePolygon(Polygon *polygon) { free(polygon->triangles); }

void drawPolygon(const Polygon *polygon, Vector2 skew, float scale,
                 Color color) {
  size_t a, b, c;
  for (size_t i = 0; i < polygon->triangles_size; i += 3) {
    a = polygon->triangles[i + 0];
    b = polygon->triangles[i + 1];
    c = polygon->triangles[i + 2];
    // NOTE(nk2ge5k): C, B, A because drawing only works in the counterclockwise
    // direction and we are storing vertices in clockwise direction.
    DrawTriangle(
        Vector2Add(Vector2Scale(polygon->vertices[c], scale), skew), // C
        Vector2Add(Vector2Scale(polygon->vertices[b], scale), skew), // B
        Vector2Add(Vector2Scale(polygon->vertices[a], scale), skew), // A
        color);
  }
}

void drawPolygonLines(const Polygon *polygon, Vector2 skew, float scale,
                      Color color) {
  Vector2 start, end;
  for (size_t i = 1; i < polygon->vertices_size; i++) {
    start = Vector2Add(Vector2Scale(polygon->vertices[i - 1], scale), skew);
    end = Vector2Add(Vector2Scale(polygon->vertices[i], scale), skew);
    DrawLineV(start, end, color);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// LINKED LIST
////////////////////////////////////////////////////////////////////////////////

typedef struct Node Node;

typedef struct Node {
  size_t idx;
  Node *prev;
  Node *next;
  Vector2 *vertex;
} Node;

// createNode allocates new node of the linked list with values zeroed out.
static Node *createNode() {
  Node *node = malloc(sizeof(struct Node));

  node->idx = 0; // NOTE(nk2ge5k): not perfect because it is a valid index
  node->next = NULL;
  node->prev = NULL;
  node->vertex = NULL;

  return node;
}

// freeNode frees the node of the linked list
// TODO(nk2ge5k): it is probably worth to allocate some sort of freelist
// beforehand, but may be not. I should think about that and measure some
// things
static void freeNode(Node *node) {
  node->prev = NULL;
  node->next = NULL;
  node->vertex = NULL;
  node->idx = 0;

  free(node);
}

// freeList frees the list and all of its nodes.
static void freeList(Node *root) {
  if (root == NULL) {
    return;
  }

  Node *node = root->next;
  Node *next;

  while (node != NULL && node != root) {
    assert(node != NULL);
    next = node->next;

    node->prev = NULL;
    node->next = NULL;

    freeNode(node);
    node = next;
  }
  root->next = NULL;

  if (node != root) {
    node = root->prev;
    while (node != NULL && node != root) {
      assert(node != NULL);
      next = node->prev;

      node->prev = NULL;
      node->next = NULL;

      freeNode(node);

      node = next;
    }
    root->prev = NULL;
  }

  freeNode(root);
}

// removeNode removes node from the linked list and frees
static void removeNode(Node *node) {
  node->prev->next = node->next;
  node->next->prev = node->prev;

  freeNode(node);
}

// swapNodes swaps two pointers to the lit nodes
static void swapNodes(Node **l, Node **r) {
  struct Node *tmp;

  tmp = *l;
  *l = *r;
  *r = tmp;
}

////////////////////////////////////////////////////////////////////////////////
/// UTILS
////////////////////////////////////////////////////////////////////////////////

// pointInTriangle checks if triangle abc contains point p
// TODO(nk2ge5k): only works on clockwise triangles.
bool pointInTriangle(const Vector2 *a, const Vector2 *b, const Vector2 *c,
                     const Vector2 *p) {
  return (c->x - p->x) * (a->y - p->y) >= (a->x - p->x) * (c->y - p->y) &&
         (a->x - p->x) * (b->y - p->y) >= (b->x - p->x) * (a->y - p->y) &&
         (b->x - p->x) * (c->y - p->y) >= (c->x - p->x) * (b->y - p->y);
}

// areaOfTriangle calculates signed or oriented area of triangle.
// area < 0 - triangle oriented in clockwise direction
// area = 0 - triangle is collinear
// area > 0 - triangle oriented in counterclockwise direction
static float areaOfTriangle(const Vector2 *a, const Vector2 *b,
                            const Vector2 *c) {
  return (b->y - a->y) * (c->x - b->x) - (b->x - a->x) * (c->y - b->y);
}

// Sign returns a sign of the floating point value.
static int32_t sign(float val) { return (0.0f < val) - (val < 0.0f); }

// polygonSign determines sign of the polygon
// sign < 0 - polygon oriented in clockwise direction
// sign = 0 - polygon is collinear
// sign > 0 - polygon oriented in counterclockwise direction
static int32_t polygonSign(struct Node *root) {
  float area = 0;
  struct Node *node;

  for (node = root->next; node != root; node = node->next) {
    area += areaOfTriangle(node->prev->vertex, // A
                           node->vertex,       // B
                           node->next->vertex  // C
    );
  }

  return sign(area);
}

////////////////////////////////////////////////////////////////////////////////
/// Eartipping
////////////////////////////////////////////////////////////////////////////////

static bool isEar(Node *node, int32_t polygon_direction) {
  Node *a = node->prev;
  Node *b = node;
  Node *c = node->next;

  float area = areaOfTriangle(a->vertex, b->vertex, c->vertex);
  int32_t direction = sign(area);
  if (area == 0 || sign(area) != polygon_direction) {
    return false;
  }

  if (direction > 0) {
    swapNodes(&a, &c);
  }

  Node *cur = node->next->next;
  for (; cur != node->prev; cur = cur->next) {
    if (pointInTriangle(a->vertex, b->vertex, c->vertex, cur->vertex)) {
      return false;
    }
  }

  return true;
}

// triangulate fills polygon with triangles using eartiping algorithm
// TODO(nk2ge5k): research different trianglulation algorithms or improve
// this one, for now it works, but it seems slow and probably not entierly
// correct.
static void triangulate(Polygon *polygon) {
  // TODO(nk2ge5k): always allocates more size then necessary
  polygon->triangles = malloc(sizeof(size_t) * polygon->vertices_size * 3);

  // Pointer to the triangle vertices
  struct Node *a, *b, *c;

  size_t ntriangles = 0;
  size_t niterations = 0;
  size_t list_len = 1;

  Node *root = createNode();
  Node *prev = root;
  Node *cur;

  root->idx = 0;
  root->vertex = &polygon->vertices[0];

  for (size_t i = 1; i < polygon->vertices_size; i++) {
    cur = createNode();

    assert(NULL != cur);

    cur->idx = i;
    cur->prev = prev;
    prev->next = cur;
    cur->vertex = &polygon->vertices[i];

    prev = cur;
    list_len++;
  }

  prev->next = root;
  root->prev = prev;

  int32_t poly_sign = polygonSign(root);

  cur = root;
  while (niterations < polygon->vertices_size * 3 && list_len >= 3 &&
         cur->next != cur->prev) {
    a = cur->prev;
    b = cur;
    c = cur->next;

    if (isEar(cur, poly_sign)) {
      polygon->triangles[ntriangles + 0] = a->idx;
      polygon->triangles[ntriangles + 1] = b->idx;
      polygon->triangles[ntriangles + 2] = c->idx;

      ntriangles += 3;

      prev = cur;
      cur = prev->next;

      removeNode(prev);

      list_len--;
    } else {
      cur = cur->next;
    }

    niterations++;
  }

  debugf("===========================\n");
  debugf("polygon size          %ld\n", polygon->vertices_size);
  debugf("number of triangles   %ld\n", ntriangles / 3);
  debugf("number of iterations  %ld\n", niterations);
  debugf("list items left       %ld\n", list_len);
  debugf("===========================\n");

  freeList(cur);

  polygon->triangles_size = ntriangles;
}
