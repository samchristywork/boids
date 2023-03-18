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

struct boid {
  float x;
  float y;
  float current_heading;
} boids[NUM_BOIDS];

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

  }
}

void simulate_boids() {

  for (int i = 0; i < NUM_BOIDS; i++) {
    // rule1(i);
    // rule2(i);
    // rule3(i);
  }

  for (int i = 0; i < NUM_BOIDS; i++) {

    float heading_weight = 1.0;

    float new_x = heading_weight * cos(boids[i].current_heading);
    float new_y = heading_weight * sin(boids[i].current_heading);

    boids[i].current_heading = atan2(new_y, new_x);

    boids[i].x += BOID_SPEED * cos(boids[i].current_heading);
    boids[i].y += BOID_SPEED * sin(boids[i].current_heading);
  }
}

int main() {
  bool debug_view = true;

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
    int delay = 1000 / 30 - (end - begin);
    if (delay > 0) {
      SDL_Delay(delay);
    }
  }
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
