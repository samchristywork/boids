#include <SDL2/SDL.h>
#include <stdbool.h>

int main() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

  SDL_Window *window =
      SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 400, 400, SDL_WINDOW_SHOWN);

  SDL_Event event;
  bool running = true;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {

      case SDL_QUIT:
        running = false;
        break;

      default:
        printf("Unhandled Event: %d\n", event.type);
        break;
      }
    }
  }
  SDL_Quit();
}
