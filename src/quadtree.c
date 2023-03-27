#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <quadtree.h>

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
        q->data[i].id = 0;
        q->data[i].x = 0;
        q->data[i].y = 0;
        quadtree_insert(q, q->data[i].id, q->data[i].x, q->data[i].y);
      }
      q->numChildren = 0;
      quadtree_insert(q, id, x, y);
    }
  }
}

void quadtree_free(struct Quadtree *q) {
  if (q->nw) {
    quadtree_free(q->nw);
    quadtree_free(q->ne);
    quadtree_free(q->sw);
    quadtree_free(q->se);

    free(q->nw);
    free(q->ne);
    free(q->sw);
    free(q->se);
  }
}

bool rect_intersects(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
                     int h2) {
  return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

int *quadtree_query(struct Quadtree *q, int x, int y, int w, int h,
                    int *length) {
  if (rect_intersects(x, y, w, h, q->x, q->y, q->w, q->h)) {
    if (q->nw) {
      int r1len, r2len, r3len, r4len;
      int *r1 = quadtree_query(q->nw, x, y, w, h, &r1len);
      int *r2 = quadtree_query(q->ne, x, y, w, h, &r2len);
      int *r3 = quadtree_query(q->sw, x, y, w, h, &r3len);
      int *r4 = quadtree_query(q->se, x, y, w, h, &r4len);
      *length = r1len + r2len + r3len + r4len;

      int *ret = malloc(sizeof(int) * (*length));

      int c = 0;
      for (int i = 0; i < r1len; i++) {
        ret[c] = r1[i];
        c++;
      }
      for (int i = 0; i < r2len; i++) {
        ret[c] = r2[i];
        c++;
      }
      for (int i = 0; i < r3len; i++) {
        ret[c] = r3[i];
        c++;
      }
      for (int i = 0; i < r4len; i++) {
        ret[c] = r4[i];
        c++;
      }

      free(r1);
      free(r2);
      free(r3);
      free(r4);

      return ret;
    }
  }
}
