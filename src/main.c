#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "quadtree.h"

#define M_PI 3.14159265358979323846
#define BOID_LENGTH 4
#define BOID_SPEED .25
#define NUM_BOIDS 1024
#define WIDTH 700
#define HEIGHT 700
#define RADIUS_MAX 20
#define RADIUS_MIN 5
#define BOID_SHADE 0x9f
#define QUADTREE_STARTING_SHADE 0x40
#define QUADTREE_SHADE_INCREMENT 0x4

struct boid {
  float x;
  float y;
  float current_heading;
  float rule1_heading;
  float rule2_heading;
  float rule3_heading;
} boids[NUM_BOIDS];

float randFloat(float low, float high) {
  return low + (high - low) * (float)rand() / (float)RAND_MAX;
}

void initialize_positions(void) {
  for (int i = 0; i < NUM_BOIDS; i++) {
    boids[i].x = randFloat(0, WIDTH);
    boids[i].y = randFloat(0, HEIGHT);
    boids[i].current_heading = randFloat(0, 3.141 * 2);
  }
}

void draw_quadtree(SDL_Renderer *renderer, struct quadtree *q, int shade,
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

    float x1 = boids[i].x;
    float y1 = boids[i].y;
    float x2 = x1 + BOID_LENGTH * cos(boids[i].current_heading);
    float y2 = y1 + BOID_LENGTH * sin(boids[i].current_heading);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    draw_quadtree(renderer, q, QUADTREE_STARTING_SHADE,
                  QUADTREE_SHADE_INCREMENT);
  }

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i == 0 && debug_view == true) {
      for (float f = 0; f < 2 * M_PI; f += .1) {
        float x1 = boids[i].x + RADIUS_MAX * cos(f);
        float y1 = boids[i].y + RADIUS_MAX * sin(f);
        float x2 = boids[i].x + RADIUS_MAX * cos(f + .1);
        float y2 = boids[i].y + RADIUS_MAX * sin(f + .1);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }
      for (float f = 0; f < 2 * M_PI; f += .1) {
        float x1 = boids[i].x + RADIUS_MIN * cos(f);
        float y1 = boids[i].y + RADIUS_MIN * sin(f);
        float x2 = boids[i].x + RADIUS_MIN * cos(f + .1);
        float y2 = boids[i].y + RADIUS_MIN * sin(f + .1);
        SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + BOID_LENGTH * cos(boids[i].rule1_heading);
        float y2 = y1 + BOID_LENGTH * sin(boids[i].rule1_heading);
        SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + BOID_LENGTH * cos(boids[i].rule2_heading);
        float y2 = y1 + BOID_LENGTH * sin(boids[i].rule2_heading);
        SDL_SetRenderDrawColor(renderer, 0, 0xff, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = boids[i].x;
        float y1 = boids[i].y;
        float x2 = x1 + BOID_LENGTH * cos(boids[i].rule3_heading);
        float y2 = y1 + BOID_LENGTH * sin(boids[i].rule3_heading);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }
    }
  }
}

float boid_dist_2(int a, int b) {
  float dx = boids[a].x - boids[b].x;
  float dy = boids[a].y - boids[b].y;

  return dx * dx + dy * dy;
}

float boid_dist(int a, int b) { return sqrt(boid_dist_2(a, b)); }

// separation: steer to avoid crowding local flockmates
void rule1(int idx) {
  boids[idx].rule1_heading = boids[idx].current_heading;

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MIN) {
        float dx = boids[idx].x - boids[i].x;
        float dy = boids[idx].y - boids[i].y;
        boids[idx].rule1_heading = atan2(dy, dx);
      }
    }
  }
}

// alignment: steer towards the average heading of local flockmates
void rule2(int idx) {
  boids[idx].rule2_heading = boids[idx].current_heading;

  float sum_x_heading = 0;
  float sum_y_heading = 0;
  int n = 0;

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_heading += cos(boids[i].current_heading);
        sum_y_heading += sin(boids[i].current_heading);
        n++;
      }
    }
  }

  if (n != 0) {
    boids[idx].rule2_heading = atan2(sum_y_heading, sum_x_heading);
  }
}

// cohesion: steer to move towards the average position (center of mass) of
// local flockmates
void rule3(int idx) {
  boids[idx].rule3_heading = boids[idx].current_heading;

  float sum_x_mass = 0;
  float sum_y_mass = 0;
  int n = 0;

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_mass += boids[i].x;
        sum_y_mass += boids[i].y;
        n++;
      }
    }
  }

  if (n != 0) {
    float dx = sum_x_mass / (float)n - boids[idx].x;
    float dy = sum_y_mass / (float)n - boids[idx].y;
    boids[idx].rule3_heading = atan2(dy, dx);
  }
}

void simulate_boids(void) {
  for (int i = 0; i < NUM_BOIDS; i++) {

    if (boids[i].x > WIDTH) {
      boids[i].x = 0;
    }
    if (boids[i].x < 0) {
      boids[i].x = WIDTH;
    }
    if (boids[i].y > HEIGHT) {
      boids[i].y = 0;
    }
    if (boids[i].y < 0) {
      boids[i].y = HEIGHT;
    }
  }

  for (int i = 0; i < NUM_BOIDS; i++) {
    rule1(i);
    rule2(i);
    rule3(i);
  }

  for (int i = 0; i < NUM_BOIDS; i++) {

    float heading_weight = 1.0;
    float rule1_weight = 0.004;
    float rule2_weight = 0.01;
    float rule3_weight = 0.0025;

    float new_x = heading_weight * cos(boids[i].current_heading) +
                  rule1_weight * cos(boids[i].rule1_heading) +
                  rule2_weight * cos(boids[i].rule2_heading) +
                  rule3_weight * cos(boids[i].rule3_heading);

    float new_y = heading_weight * sin(boids[i].current_heading) +
                  rule1_weight * sin(boids[i].rule1_heading) +
                  rule2_weight * sin(boids[i].rule2_heading) +
                  rule3_weight * sin(boids[i].rule3_heading);

    boids[i].current_heading = atan2(new_y, new_x);

    boids[i].x += BOID_SPEED * cos(boids[i].current_heading);
    boids[i].y += BOID_SPEED * sin(boids[i].current_heading);
  }
}

int main(void) {
  bool debug_view = true;
  bool cap_framerate = true;
  int frame = 0;

  srand(time(0));

  initialize_positions();

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();

  SDL_Window *window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                                        SDL_WINDOW_SHOWN);

  SDL_Surface *surface = SDL_LoadBMP("test.bmp");

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  SDL_FreeSurface(surface);
  surface = NULL;

  TTF_Font *font = TTF_OpenFont(
      "/usr/share/fonts/liberation/LiberationSans-Regular.ttf", 24);

  SDL_Color white = {255, 255, 255};

  SDL_Event event;
  bool running = true;
  while (running) {
    Uint32 begin = SDL_GetTicks();

    while (SDL_PollEvent(&event)) {
      switch (event.type) {

      case SDL_QUIT:
        running = false;
        break;

      case SDL_KEYDOWN:

        switch (event.key.keysym.sym) {

        case SDLK_ESCAPE:
          running = false;
          break;

        case SDLK_SPACE:
          cap_framerate = !cap_framerate;
          break;

        default:
          printf("Unhandled Key: %d\n", event.key.keysym.sym);
          break;
        }
        break;

      case SDL_POLLSENTINEL:
        break;

      default:
        printf("Unhandled Event: %d\n", event.type);
        break;
      }
    }

    int shade = 0x07;
    SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
    SDL_RenderClear(renderer);


    shade = 0xff;
    SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);


    draw_boids(renderer, debug_view);

    simulate_boids();

    char frame_text[256];
    snprintf(frame_text, 255, "%d", frame);
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, frame_text, white);
    SDL_Texture *textTexture =
        SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &textSurface->clip_rect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_RenderPresent(renderer);

    Uint32 end = SDL_GetTicks();
    if (cap_framerate) {
      int delay = 1000 / 60 - (end - begin);
      if (delay > 0) {
        SDL_Delay(delay);
      }
    }

    frame++;
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
