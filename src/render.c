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

    if (i == 0 && debug_view == true && num_boids > 0) {
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

void draw_checkbox(SDL_Renderer *renderer, TTF_Font *font, int w, int h,
                   struct Widget *widget) {
  SDL_Rect r;
  int shade;
  int width = 20;
  int height = 20;
  int padding = 10;

  // Outer
  r.x = padding;
  r.y = h - padding - height;
  r.w = width;
  r.h = height;
  shade = 0x70;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &r);
  widget->minx = r.x;
  widget->miny = r.y;
  widget->width = r.w;
  widget->height = r.h;

  // Border
  r.x = padding;
  r.y = h - padding - height;
  r.w = width;
  r.h = height;
  shade = 0xd0;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderDrawRect(renderer, &r);

  // Inner
  if (widget->value_b) {
    int foo = 6;
    r.x = padding + foo;
    r.y = h - padding - height + foo;
    r.w = width - foo * 2;
    r.h = height - foo * 2;
    shade = 0xd0;
    SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
    SDL_RenderFillRect(renderer, &r);
  }

  SDL_Color white = {0xff, 0xff, 0xff, 0xff};

  // Value text
  char buf[100];
  if (widget->value_b) {
    snprintf(buf, 100, "%s: True", widget->name);
  } else {
    snprintf(buf, 100, "%s: False", widget->name);
  }
  draw_text(renderer, font, 40, h - padding - height / 2 - 6, white, buf);
}

void draw_slider(SDL_Renderer *renderer, TTF_Font *font, int w, int h,
                 struct Widget *widget) {
  SDL_Rect r;
  int shade;
  int width = 200;
  int height = 20;
  int padding = 10;
  int inner_padding = 50;

  float relative_value =
      (widget->value_f - widget->min) / (widget->max - widget->min);

  // Outer
  r.x = padding;
  r.y = h - padding - height;
  r.w = width;
  r.h = height;
  shade = 0x70;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &r);
  widget->miny = r.y;
  widget->height = r.h;

  // Border
  r.x = padding;
  r.y = h - padding - height;
  r.w = width;
  r.h = height;
  shade = 0xd0;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderDrawRect(renderer, &r);

  // Inner
  r.x = padding + inner_padding;
  r.y = h - padding - height + height / 2 - 2;
  r.w = width - inner_padding * 2;
  r.h = 4;
  shade = 0xd0;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &r);
  widget->width = r.w;
  widget->minx = r.x;

  // Handle
  r.x = padding + inner_padding + relative_value * (width - inner_padding * 2);
  r.y = h - padding - height + height / 2 - 4;
  r.w = 4;
  r.h = 8;
  shade = 0xd0;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &r);

  SDL_Color white = {0xff, 0xff, 0xff, 0xff};

  // Left text
  char buf[100];
  snprintf(buf, 10, "%4.4f", widget->min);
  draw_text(renderer, font, padding + 4, h - padding - height / 2 - 6, white,
            buf);

  // Right text
  snprintf(buf, 10, "%4.4f", widget->max);
  draw_text(renderer, font, padding + width - 42, h - padding - height / 2 - 6,
            white, buf);

  // Value text
  snprintf(buf, 100, "%s: %4.4f", widget->name, widget->value_f);
  draw_text(renderer, font, 225, h - padding - height / 2 - 6, white, buf);
}

void render(SDL_Renderer *renderer, SDL_Window *window, struct Boid *boids,
            int num_boids, struct Widget *widgets, int num_widgets,
            struct Context parent, struct Context child, int frame, int fps,
            SDL_Color white, struct Quadtree *q, TTF_Font *font,
            bool debug_view) {

  int w;
  int h;
  SDL_GetWindowSize(window, &w, &h);

  int shade = 0x07;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderClear(renderer);

  draw_boids(renderer, boids, num_boids, parent, child, debug_view, q);

  char frame_text[256];
  snprintf(frame_text, 255, "Frame: %d", frame);
  draw_text(renderer, font, 5, 5, white, frame_text);

  char framerate_text[256];
  snprintf(framerate_text, 255, "FPS: %d", (int)fps);
  draw_text(renderer, font, 5, 16 + 5, white, framerate_text);

  char num_boids_text[256];
  snprintf(num_boids_text, 255, "Boids: %d", (int)num_boids);
  draw_text(renderer, font, 5, 32 + 5, white, num_boids_text);

  for (int i = 0; i < num_widgets; i++) {
    if (widgets[i].type == WIDGET_SLIDER) {
      draw_slider(renderer, font, w, h - 30 * i, &widgets[i]);
    } else if (widgets[i].type == WIDGET_CHECKBOX) {
      draw_checkbox(renderer, font, w, h - 30 * i, &widgets[i]);
    }
  }

  SDL_RenderPresent(renderer);
}
