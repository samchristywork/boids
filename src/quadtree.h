#ifndef QUADTREE_H
#define QUADTREE_H

#define QUADTREE_MAX_CHILDREN 1

struct quadtree_point {
  float x;
  float y;
  int id;
};

struct quadtree {
  float x;
  float y;
  float w;
  float h;

  struct quadtree *nw;
  struct quadtree *ne;
  struct quadtree *sw;
  struct quadtree *se;

  struct quadtree_point data[QUADTREE_MAX_CHILDREN];
  int num_children;
};

void quadtree_insert(struct quadtree *q, int id, float x, float y);

int *quadtree_query(float x, float y, float w, float h);

#endif
