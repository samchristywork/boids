#include <SDL2/SDL2_gfxPrimitives.h>

#include <main.h>
#include <render.h>

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

void draw_quadtree(SDL_Renderer *renderer, struct Quadtree *q, int shade,
                   int shade_increment) {

  SDL_Rect rect;
  rect.x = q->x;
  rect.y = q->y;
  rect.w = q->w;
  rect.h = q->h;

  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &rect);
  shade -= shade_increment;
  if (shade < 0) {
    shade = 0;
  }

  if (q->nw) {
    draw_quadtree(renderer, q->nw, shade, shade_increment);
  }

  if (q->ne) {
    draw_quadtree(renderer, q->ne, shade, shade_increment);
  }

  if (q->sw) {
    draw_quadtree(renderer, q->sw, shade, shade_increment);
  }

  if (q->se) {
    draw_quadtree(renderer, q->se, shade, shade_increment);
  }
}

void draw_boid(SDL_Renderer *renderer, struct Boid *boid, int id) {
  float cx = boid->x;
  float cy = boid->y;

  float current_heading = boid->current_heading;

  float x1 = cx + BOID_LENGTH * cos(current_heading);
  float y1 = cy + BOID_LENGTH * sin(current_heading);

  float x2 = cx + BOID_LENGTH * cos(current_heading + M_PI / 2) * 0.25;
  float y2 = cy + BOID_LENGTH * sin(current_heading + M_PI / 2) * 0.25;

  float x3 = cx + BOID_LENGTH * cos(current_heading - M_PI / 2) * 0.25;
  float y3 = cy + BOID_LENGTH * sin(current_heading - M_PI / 2) * 0.25;

  filledTrigonRGBA(renderer, x1, y1, x2, y2, x3, y3, BOID_SHADE, BOID_SHADE,
                   BOID_SHADE, 0xff);
}

void draw_boids(SDL_Renderer *renderer, struct Boid boids[], int num_boids,
                bool debug_view, struct Quadtree *q) {
  if (debug_view) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    draw_quadtree(renderer, q, QUADTREE_STARTING_SHADE,
                  QUADTREE_SHADE_INCREMENT);
  }

  for (int i = 0; i < num_boids; i++) {
    draw_boid(renderer, &boids[i], i);

    if (i == 0 && debug_view == true) {
      aacircleRGBA(renderer, boids[i].x, boids[i].y, RADIUS_MAX, 0xff, 0xff,
                   0xff, 0xff);

      aacircleRGBA(renderer, boids[i].x, boids[i].y, RADIUS_MIN, 0xff, 0xff,
                   0xff, 0xff);

      int ind_len = 10;
      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + ind_len * cos(boids[i].rule1_heading);
        float y2 = y1 + ind_len * sin(boids[i].rule1_heading);
        SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + ind_len * cos(boids[i].rule2_heading);
        float y2 = y1 + ind_len * sin(boids[i].rule2_heading);
        SDL_SetRenderDrawColor(renderer, 0, 0xff, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + ind_len * cos(boids[i].rule3_heading);
        float y2 = y1 + ind_len * sin(boids[i].rule3_heading);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }
    }
  }
}
