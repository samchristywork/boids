#include <SDL2/SDL2_gfxPrimitives.h>

#include <main.h>
#include <render.h>

void transform_to_context(struct Context *parent, struct Context *new, float *x,
                          float *y) {
  if (x) {
    *x += new->x - parent->x;
    *x /= new->w / parent->w;
  }

  if (y) {
    *y += new->y - parent->y;
    *y /= new->h / parent->h;
  }
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, int x, int y,
               SDL_Color color, char *text) {
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
  SDL_Texture *textTexture =
      SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_Rect rect = textSurface->clip_rect;
  rect.x = x;
  rect.y = y;
  SDL_RenderCopy(renderer, textTexture, NULL, &rect);
  SDL_FreeSurface(textSurface);
  SDL_DestroyTexture(textTexture);
}

void draw_quadtree(SDL_Renderer *renderer, struct Quadtree *q,
                   struct Context parent, struct Context child, int shade,
                   int shade_increment) {

  float x1 = q->x;
  float y1 = q->y;
  float x2 = q->x + q->w;
  float y2 = q->y + q->h;

  transform_to_context(&parent, &child, &x1, &y1);
  transform_to_context(&parent, &child, &x2, &y2);

  SDL_Rect rect;
  rect.x = x1;
  rect.y = y1;
  rect.w = x2 - x1;
  rect.h = y2 - y1;

  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &rect);
  shade -= shade_increment;
  if (shade < 0) {
    shade = 0;
  }

  if (q->nw) {
    draw_quadtree(renderer, q->nw, parent, child, shade, shade_increment);
  }

  if (q->ne) {
    draw_quadtree(renderer, q->ne, parent, child, shade, shade_increment);
  }

  if (q->sw) {
    draw_quadtree(renderer, q->sw, parent, child, shade, shade_increment);
  }

  if (q->se) {
    draw_quadtree(renderer, q->se, parent, child, shade, shade_increment);
  }
}

void draw_boid(SDL_Renderer *renderer, struct Boid *boid, struct Context parent,
               struct Context child, int id) {
  float cx = boid->x;
  float cy = boid->y;

  float currentHeading = boid->currentHeading;

  float x1 = cx + BOID_LENGTH * cos(currentHeading);
  float y1 = cy + BOID_LENGTH * sin(currentHeading);

  float x2 = cx + BOID_LENGTH * cos(currentHeading + M_PI / 2) * 0.25;
  float y2 = cy + BOID_LENGTH * sin(currentHeading + M_PI / 2) * 0.25;

  float x3 = cx + BOID_LENGTH * cos(currentHeading - M_PI / 2) * 0.25;
  float y3 = cy + BOID_LENGTH * sin(currentHeading - M_PI / 2) * 0.25;

  transform_to_context(&parent, &child, &x1, &y1);
  transform_to_context(&parent, &child, &x2, &y2);
  transform_to_context(&parent, &child, &x3, &y3);

  filledTrigonRGBA(renderer, x1, y1, x2, y2, x3, y3, BOID_SHADE, BOID_SHADE,
                   BOID_SHADE, 0xff);
}

void draw_boids(SDL_Renderer *renderer, struct Boid boids[], int num_boids,
                struct Context parent, struct Context child, bool debug_view,
                struct Quadtree *q) {
  if (debug_view) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    draw_quadtree(renderer, q, parent, child, QUADTREE_STARTING_SHADE,
                  QUADTREE_SHADE_INCREMENT);
  }

  for (int i = 0; i < num_boids; i++) {
    draw_boid(renderer, &boids[i], parent, child, i);

    if (i == 0 && debug_view == true) {
      float x = boids[i].x;
      float y = boids[i].y;
      transform_to_context(&parent, &child, &x, &y);

      float scale = parent.w / child.w;

      aacircleRGBA(renderer, x, y, scale * RADIUS_MAX, 0xff, 0xff, 0xff, 0xff);
      aacircleRGBA(renderer, x, y, scale * RADIUS_MIN, 0xff, 0xff, 0xff, 0xff);

      int ind_len = 10;
      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + ind_len * cos(boids[i].headings[0]);
        float y2 = y1 + ind_len * sin(boids[i].headings[0]);

        transform_to_context(&parent, &child, &x1, &y1);
        transform_to_context(&parent, &child, &x2, &y2);
        SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + ind_len * cos(boids[i].headings[1]);
        float y2 = y1 + ind_len * sin(boids[i].headings[1]);

        transform_to_context(&parent, &child, &x1, &y1);
        transform_to_context(&parent, &child, &x2, &y2);
        SDL_SetRenderDrawColor(renderer, 0, 0xff, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + ind_len * cos(boids[i].headings[2]);
        float y2 = y1 + ind_len * sin(boids[i].headings[2]);

        transform_to_context(&parent, &child, &x1, &y1);
        transform_to_context(&parent, &child, &x2, &y2);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }
    }
  }
}
