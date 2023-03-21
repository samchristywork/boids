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

float g_fps = 0;

struct Boid {
  float x;
  float y;
  float current_heading;
  float rule1_heading;
  float rule2_heading;
  float rule3_heading;
  float rule4_heading;
};

struct Boid g_boids[NUM_BOIDS];

float random_float(float low, float high) {
  return low + (high - low) * (float)rand() / (float)RAND_MAX;
}

void initialize_positions(void) {
  for (int i = 0; i < NUM_BOIDS; i++) {
    g_boids[i].x = random_float(0, WIDTH);
    g_boids[i].y = random_float(0, HEIGHT);
    g_boids[i].current_heading = random_float(0, 3.141 * 2);
  }
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

void draw_boid(SDL_Renderer *renderer, int id) {
  float cx = g_boids[id].x;
  float cy = g_boids[id].y;

  float current_heading = g_boids[id].current_heading;

  float x1 = cx + BOID_LENGTH * cos(current_heading);
  float y1 = cy + BOID_LENGTH * sin(current_heading);

  float x2 = cx + BOID_LENGTH * cos(current_heading + M_PI / 2) * 0.25;
  float y2 = cy + BOID_LENGTH * sin(current_heading + M_PI / 2) * 0.25;

  float x3 = cx + BOID_LENGTH * cos(current_heading - M_PI / 2) * 0.25;
  float y3 = cy + BOID_LENGTH * sin(current_heading - M_PI / 2) * 0.25;

  filledTrigonRGBA(renderer, x1, y1, x2, y2, x3, y3, BOID_SHADE, BOID_SHADE,
                   BOID_SHADE, 0xff);
}

void draw_boids(SDL_Renderer *renderer, bool debug_view, struct Quadtree *q) {
  if (debug_view) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    draw_quadtree(renderer, q, QUADTREE_STARTING_SHADE,
                  QUADTREE_SHADE_INCREMENT);
  }

  for (int i = 0; i < NUM_BOIDS; i++) {
    draw_boid(renderer, i);

    if (i == 0 && debug_view == true) {
      aacircleRGBA(renderer, g_boids[i].x, g_boids[i].y, RADIUS_MAX, 0xff, 0xff,
                   0xff, 0xff);

      aacircleRGBA(renderer, g_boids[i].x, g_boids[i].y, RADIUS_MIN, 0xff, 0xff,
                   0xff, 0xff);

      int ind_len = 10;
      {
        float x1 = g_boids[i].x;
        float y1 = g_boids[i].y;
        float x2 = x1 + ind_len * cos(g_boids[i].rule1_heading);
        float y2 = y1 + ind_len * sin(g_boids[i].rule1_heading);
        SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = g_boids[i].x;
        float y1 = g_boids[i].y;
        float x2 = x1 + ind_len * cos(g_boids[i].rule2_heading);
        float y2 = y1 + ind_len * sin(g_boids[i].rule2_heading);
        SDL_SetRenderDrawColor(renderer, 0, 0xff, 0, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }

      {
        float x1 = g_boids[i].x;
        float y1 = g_boids[i].y;
        float x2 = x1 + ind_len * cos(g_boids[i].rule3_heading);
        float y2 = y1 + ind_len * sin(g_boids[i].rule3_heading);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0xff, 0xff);
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
      }
    }
  }
}

float boid_dist_2(int a, int b) {
  float dx = g_boids[a].x - g_boids[b].x;
  float dy = g_boids[a].y - g_boids[b].y;

  return dx * dx + dy * dy;
}

float boid_dist(int a, int b) { return sqrt(boid_dist_2(a, b)); }

// separation: steer to avoid crowding local flockmates
void rule1(int idx) {
  g_boids[idx].rule1_heading = g_boids[idx].current_heading;

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MIN) {
        float dx = g_boids[idx].x - g_boids[i].x;
        float dy = g_boids[idx].y - g_boids[i].y;
        g_boids[idx].rule1_heading = atan2(dy, dx);
      }
    }
  }
}

// alignment: steer towards the average heading of local flockmates
void rule2(int idx) {
  g_boids[idx].rule2_heading = g_boids[idx].current_heading;

  float sum_x_heading = 0;
  float sum_y_heading = 0;
  int n = 0;

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_heading += cos(g_boids[i].current_heading);
        sum_y_heading += sin(g_boids[i].current_heading);
        n++;
      }
    }
  }

  if (n != 0) {
    g_boids[idx].rule2_heading = atan2(sum_y_heading, sum_x_heading);
  }
}

// cohesion: steer to move towards the average position (center of mass) of
// local flockmates
void rule3(int idx) {
  g_boids[idx].rule3_heading = g_boids[idx].current_heading;

  float sum_x_mass = 0;
  float sum_y_mass = 0;
  int n = 0;

  for (int i = 0; i < NUM_BOIDS; i++) {
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_mass += g_boids[i].x;
        sum_y_mass += g_boids[i].y;
        n++;
      }
    }
  }

  if (n != 0) {
    float dx = sum_x_mass / (float)n - g_boids[idx].x;
    float dy = sum_y_mass / (float)n - g_boids[idx].y;
    g_boids[idx].rule3_heading = atan2(dy, dx);
  }
}

// noise: steer in random directions
void rule4(int idx) {
  g_boids[idx].rule4_heading = g_boids[idx].current_heading;

  g_boids[idx].rule4_heading += random_float(-0.1, 0.1);
}

void simulate_boids(void) {

  for (int i = 0; i < NUM_BOIDS; i++) {
    g_boids[i].x += BOID_SPEED * cos(g_boids[i].current_heading);
    g_boids[i].y += BOID_SPEED * sin(g_boids[i].current_heading);
  }

  for (int i = 0; i < NUM_BOIDS; i++) {

    if (g_boids[i].x > WIDTH) {
      g_boids[i].x = 0;
    }
    if (g_boids[i].x < 0) {
      g_boids[i].x = WIDTH;
    }
    if (g_boids[i].y > HEIGHT) {
      g_boids[i].y = 0;
    }
    if (g_boids[i].y < 0) {
      g_boids[i].y = HEIGHT;
    }
  }

  for (int i = 0; i < NUM_BOIDS; i++) {
    rule1(i);
    rule2(i);
    rule3(i);
    rule4(i);
  }

  for (int i = 0; i < NUM_BOIDS; i++) {

    float heading_weight = 1.0;
    float rule1_weight = 0.04;
    float rule2_weight = 0.01;
    float rule3_weight = 0.0025;
    float rule4_weight = 0.0;

    float new_x = heading_weight * cos(g_boids[i].current_heading) +
                  rule1_weight * cos(g_boids[i].rule1_heading) +
                  rule2_weight * cos(g_boids[i].rule2_heading) +
                  rule3_weight * cos(g_boids[i].rule3_heading) +
                  rule4_weight * cos(g_boids[i].rule4_heading);

    float new_y = heading_weight * sin(g_boids[i].current_heading) +
                  rule1_weight * sin(g_boids[i].rule1_heading) +
                  rule2_weight * sin(g_boids[i].rule2_heading) +
                  rule3_weight * sin(g_boids[i].rule3_heading) +
                  rule4_weight * sin(g_boids[i].rule4_heading);

    g_boids[i].current_heading = atan2(new_y, new_x);

    g_boids[i].x += BOID_SPEED * cos(g_boids[i].current_heading);
    g_boids[i].y += BOID_SPEED * sin(g_boids[i].current_heading);
  }
}

int main(void) {
  bool cap_framerate = false;
  bool debug_view = true;
  bool paused = false;

  int frame = 0;

  srand(time(0));

  initialize_positions();

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();

  SDL_Window *window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                                        SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  TTF_Font *font = TTF_OpenFont("res/LiberationSans-Regular.ttf", 12);

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

        case SDLK_c:
          cap_framerate = !cap_framerate;
          break;

        case SDLK_d:
          debug_view = !debug_view;
          break;

        case SDLK_SPACE:
          paused = !paused;
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

    struct Quadtree q = {0};
    q.w = WIDTH;
    q.h = HEIGHT;

    for (int i = 0; i < NUM_BOIDS; i++) {
      quadtree_insert(&q, i, g_boids[i].x, g_boids[i].y);
    }

    draw_boids(renderer, debug_view, &q);

    if (!paused) {
      simulate_boids();
      frame++;
    }

    char frame_text[256];
    snprintf(frame_text, 255, "%d", frame);
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, frame_text, white);
    SDL_Texture *textTexture =
        SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect rect = textSurface->clip_rect;
    rect.x = 0;
    rect.y = 0;
    SDL_RenderCopy(renderer, textTexture, NULL, &rect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    char framerate_text[256];
    snprintf(framerate_text, 255, "%d", (int)g_fps);
    textSurface = TTF_RenderText_Solid(font, framerate_text, white);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    rect = textSurface->clip_rect;
    rect.x = 0;
    rect.y = 16;
    SDL_RenderCopy(renderer, textTexture, NULL, &rect);
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

    if (frame % 10 == 0) {
      g_fps = 1000.0 / (SDL_GetTicks() - begin);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
