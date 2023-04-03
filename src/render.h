#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include <main.h>
#include <quadtree.h>

#define BOID_LENGTH 4
#define BOID_SPEED .25
#define MAX_BOIDS 10000
#define WIDTH 1200
#define HEIGHT 700
#define RADIUS_MAX 20
#define RADIUS_MIN 5
#define BOID_SHADE 0x9f
#define QUADTREE_STARTING_SHADE 0x40
#define QUADTREE_SHADE_INCREMENT 0x4

struct Context {
  float x;
  float y;
  float w;
  float h;
};

void draw_text(SDL_Renderer *renderer, TTF_Font *font, int x, int y,
               SDL_Color color, char *text);

void draw_quadtree(SDL_Renderer *renderer, struct Quadtree *q,
                   struct Context parent, struct Context child, int shade,
                   int shade_increment);

void draw_boid(SDL_Renderer *renderer, struct Boid *boid, struct Context parent,
               struct Context child, int id);

void draw_boids(SDL_Renderer *renderer, struct Boid boids[], int num_boids,
                struct Context parent, struct Context child, bool debug_view,
                struct Quadtree *q);

#endif
