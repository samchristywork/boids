#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quadtree.h"

void quadtree_insert(struct Quadtree *q, int id, float x, float y) {

  if (q->nw != NULL) {
    // West
    if (x < q->x + q->w / 2) {
      // Northwest
      if (y < q->y + q->h / 2) {
        quadtree_insert(q->nw, id, x, y);

        // Southwest
      } else {
        quadtree_insert(q->sw, id, x, y);
      }

      // East
    } else {
      // Northeast
      if (y < q->y + q->h / 2) {
        quadtree_insert(q->ne, id, x, y);

        // Southeast
      } else {
        quadtree_insert(q->se, id, x, y);
      }
    }
  } else {

    if (q->numChildren < QUADTREE_MAX_CHILDREN) {
      q->data[q->numChildren].id = id;
      q->data[q->numChildren].x = x;
      q->data[q->numChildren].y = y;
      q->numChildren++;
    } else {

      q->nw = malloc(sizeof(struct Quadtree));
      q->ne = malloc(sizeof(struct Quadtree));
      q->sw = malloc(sizeof(struct Quadtree));
      q->se = malloc(sizeof(struct Quadtree));

      memset(q->nw, 0, sizeof(struct Quadtree));
      memset(q->ne, 0, sizeof(struct Quadtree));
      memset(q->sw, 0, sizeof(struct Quadtree));
      memset(q->se, 0, sizeof(struct Quadtree));

      q->nw->x = q->x;
      q->nw->y = q->y;
      q->nw->w = q->w / 2;
      q->nw->h = q->h / 2;

      q->ne->x = q->x + q->w / 2;
      q->ne->y = q->y;
      q->ne->w = q->w / 2;
      q->ne->h = q->h / 2;

      q->sw->x = q->x;
      q->sw->y = q->y + q->h / 2;
      q->sw->w = q->w / 2;
      q->sw->h = q->h / 2;

      q->se->x = q->x + q->w / 2;
      q->se->y = q->y + q->h / 2;
      q->se->w = q->w / 2;
      q->se->h = q->h / 2;

      for (int i = 0; i < QUADTREE_MAX_CHILDREN; i++) {
        quadtree_insert(q, q->data[i].id, q->data[i].x, q->data[i].y);
      }
      quadtree_insert(q, id, x, y);
    }
  }
}
