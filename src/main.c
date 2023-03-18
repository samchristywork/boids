#include <SDL2/SDL.h>
#include <stdbool.h>

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

int main() {

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

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_Rect rectToDraw = {100, 100, 100, 100};

    SDL_SetRenderDrawColor(renderer, 0xff, 0, 0, 0);
    SDL_RenderFillRect(renderer, &rectToDraw);

    SDL_SetRenderDrawColor(renderer, 0, 0xff, 0, 0);
    SDL_RenderDrawRect(renderer, &rectToDraw);

    SDL_RenderDrawLine(renderer, 0, 0, 100, 100);

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
