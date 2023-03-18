#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define M_PI 3.14159265358979323846
#define BOID_LENGTH 4
#define BOID_SPEED .25
#define NUM_BOIDS 256
#define WIDTH 300
#define HEIGHT 300
#define RADIUS_MAX 20
#define RADIUS_MIN 5

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

void initialize_positions() {
  for (int i = 0; i < NUM_BOIDS; i++) {
    boids[i].x = randFloat(0, WIDTH);
    boids[i].y = randFloat(0, HEIGHT);
    boids[i].current_heading = randFloat(0, 3.141 * 2);
  }
}

void draw_boids(SDL_Renderer *renderer, bool debug_view) {
  for (int i = 0; i < NUM_BOIDS; i++) {
    boids[i].x += BOID_SPEED * cos(boids[i].current_heading);
    boids[i].y += BOID_SPEED * sin(boids[i].current_heading);

    float x1 = boids[i].x;
    float y1 = boids[i].y;
    float x2 = x1 + BOID_LENGTH * cos(boids[i].current_heading);
    float y2 = y1 + BOID_LENGTH * sin(boids[i].current_heading);
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

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

// TODO: less efficient
float boid_dist(int a, int b) { return sqrt(boid_dist_2(a, b)); }

// separation: steer to avoid crowding local flockmates
void rule1(int idx) {
  boids[idx].rule1_heading = boids[idx].current_heading;
}

// alignment: steer towards the average heading of local flockmates
void rule2(int idx) {
  boids[idx].rule2_heading = boids[idx].current_heading;
}

// cohesion: steer to move towards the average position (center of mass) of
// local flockmates
void rule3(int idx) {
  boids[idx].rule3_heading = boids[idx].current_heading;
}

void simulate_boids() {
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
    float rule1_weight = 0.0;
    float rule2_weight = 0.0;
    float rule3_weight = 0.0;

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

int main() {
  bool debug_view = true;
  bool cap_framerate = true;

  srand(time(0));

  initialize_positions();

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  SDL_Window *window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                                        SDL_WINDOW_SHOWN);

  SDL_Surface *surface = SDL_LoadBMP("test.bmp");

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

  SDL_FreeSurface(surface);
  surface = NULL;

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

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);




    draw_boids(renderer, debug_view);

    simulate_boids();

    SDL_RenderPresent(renderer);

    Uint32 end = SDL_GetTicks();
    if (cap_framerate) {
      int delay = 1000 / 60 - (end - begin);
      if (delay > 0) {
        SDL_Delay(delay);
      }
    }
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
