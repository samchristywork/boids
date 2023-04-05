#ifndef QUADTREE_H
#define QUADTREE_H

#define QUADTREE_MAX_CHILDREN 1

struct QuadtreePoint {
  float x;
  float y;
  int id;
};

struct Quadtree {
  float x;
  float y;
  float w;
  float h;

  struct Quadtree *nw;
  struct Quadtree *ne;
  struct Quadtree *sw;
  struct Quadtree *se;

  struct QuadtreePoint data[QUADTREE_MAX_CHILDREN];
  int numChildren;
};

void quadtree_insert(struct Quadtree *q, int id, float x, float y);

void quadtree_free(struct Quadtree *q);

int *quadtree_query(struct Quadtree *q, int x, int y, int w, int h,
                    int *length);

#endif
