#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <command_line.h>
#include <main.h>
#include <quadtree.h>
#include <render.h>

void draw_slider(SDL_Renderer *renderer, TTF_Font *font, int w, int h,
                 float min, float max, float value) {
  SDL_Rect r;
  int shade;
  int width = 200;
  int height = 20;
  int padding = 10;
  int inner_padding = 40;

  float relative_value = value / (max - min) - max;

  // Outer
  r.x = padding;
  r.y = h - padding - height;
  r.w = width;
  r.h = height;
  shade = 0x70;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderFillRect(renderer, &r);

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
  snprintf(buf, 10, "%2.2f", min);
  draw_text(renderer, font, padding + 4, h - padding - height / 2 - 6, white,
            buf);

  // Right text
  snprintf(buf, 10, "%2.2f", max);
  draw_text(renderer, font, padding + width - 28, h - padding - height / 2 - 6,
            white, buf);

  snprintf(buf, 100, "Value: %2.2f", value);
  draw_text(renderer, font, 225, h - padding - height / 2 - 6, white, buf);
}

void render(SDL_Renderer *renderer, SDL_Window *window, struct Boid *boids,
            int num_boids, int frame, int fps, SDL_Color white,
            struct Quadtree *q, TTF_Font *font, bool debug_view) {

  int w;
  int h;
  SDL_GetWindowSize(window, &w, &h);

  int shade = 0x07;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderClear(renderer);

  draw_boids(renderer, boids, num_boids, debug_view, q);

  char frame_text[256];
  snprintf(frame_text, 255, "Frame: %d", frame);
  draw_text(renderer, font, 0, 0, white, frame_text);

  char framerate_text[256];
  snprintf(framerate_text, 255, "FPS: %d", (int)fps);
  draw_text(renderer, font, 0, 16, white, framerate_text);

  char num_boids_text[256];
  snprintf(num_boids_text, 255, "Boids: %d", (int)num_boids);
  draw_text(renderer, font, 0, 32, white, num_boids_text);

  draw_slider(renderer, font, w, h, 0.5, 1.0, 0.75);

  SDL_RenderPresent(renderer);
}

float random_float(float low, float high) {
  return low + (high - low) * (float)rand() / (float)RAND_MAX;
}

void add_boid(struct Boid *boids, int *num_boids) {
  if (*num_boids < MAX_BOIDS) {
    boids[*num_boids].x = random_float(0, WIDTH);
    boids[*num_boids].y = random_float(0, HEIGHT);
    boids[*num_boids].currentHeading = random_float(0, 3.141 * 2);
    (*num_boids)++;
  }
}

void remove_boid(int *num_boids) { (*num_boids)--; }

int initialize_positions(struct Boid *boids, int n) {
  int num_boids = 0;
  for (int i = 0; i < n; i++) {
    add_boid(boids, &num_boids);
  }
  return num_boids;
}

float boid_dist_2(struct Boid *boids, int a, int b) {
  float dx = boids[a].x - boids[b].x;
  float dy = boids[a].y - boids[b].y;

  return dx * dx + dy * dy;
}

float boid_dist(struct Boid *boids, int a, int b) {
  return sqrt(boid_dist_2(boids, a, b));
}

// separation: steer to avoid crowding local flockmates
void rule1(struct Boid *boids, int idx, struct Quadtree *q) {
  boids[idx].headings[0] = boids[idx].currentHeading;

  int length;
  int *nearby = quadtree_query(q, boids[idx].x - RADIUS_MIN / 2.0,
                               boids[idx].y - RADIUS_MIN / 2.0, RADIUS_MIN,
                               RADIUS_MIN, &length);

  for (int j = 0; j < length; j++) {
    int i = nearby[j];
    if (i != idx) {
      float dist = boid_dist(boids, idx, i);
      if (dist < RADIUS_MIN) {
        float dx = boids[idx].x - boids[i].x;
        float dy = boids[idx].y - boids[i].y;
        boids[idx].headings[0] = atan2(dy, dx);
      }
    }
  }

  free(nearby);
}

// alignment: steer towards the average heading of local flockmates
void rule2(struct Boid *boids, int idx, struct Quadtree *q) {
  boids[idx].headings[1] = boids[idx].currentHeading;

  float sum_x_heading = 0;
  float sum_y_heading = 0;
  int n = 0;

  int length;
  int *nearby = quadtree_query(q, boids[idx].x - RADIUS_MAX / 2.0,
                               boids[idx].y - RADIUS_MAX / 2.0, RADIUS_MAX,
                               RADIUS_MAX, &length);

  for (int j = 0; j < length; j++) {
    int i = nearby[j];
    if (i != idx) {
      float dist = boid_dist(boids, idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_heading += cos(boids[i].currentHeading);
        sum_y_heading += sin(boids[i].currentHeading);
        n++;
      }
    }
  }

  free(nearby);

  if (n != 0) {
    boids[idx].headings[1] = atan2(sum_y_heading, sum_x_heading);
  }
}

// cohesion: steer to move towards the average position (center of mass) of
// local flockmates
void rule3(struct Boid *boids, int idx, struct Quadtree *q) {
  boids[idx].headings[2] = boids[idx].currentHeading;

  float sum_x_mass = 0;
  float sum_y_mass = 0;
  int n = 0;

  int length;
  int *nearby = quadtree_query(q, boids[idx].x - RADIUS_MAX / 2.0,
                               boids[idx].y - RADIUS_MAX / 2.0, RADIUS_MAX,
                               RADIUS_MAX, &length);

  for (int j = 0; j < length; j++) {
    int i = nearby[j];
    if (i != idx) {
      float dist = boid_dist(boids, idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_mass += boids[i].x;
        sum_y_mass += boids[i].y;
        n++;
      }
    }
  }

  free(nearby);

  if (n != 0) {
    float dx = sum_x_mass / (float)n - boids[idx].x;
    float dy = sum_y_mass / (float)n - boids[idx].y;
    boids[idx].headings[2] = atan2(dy, dx);
  }
}

// noise: steer in random directions
void rule4(struct Boid *boids, int idx, struct Quadtree *q) {
  boids[idx].headings[3] = boids[idx].currentHeading;

  boids[idx].headings[3] += random_float(-0.1, 0.1);
}


void simulate_boids(struct Boid *boids, int num_boids, struct Quadtree *q) {

  for (int i = 0; i < num_boids; i++) {
    boids[i].x += BOID_SPEED * cos(boids[i].currentHeading);
    boids[i].y += BOID_SPEED * sin(boids[i].currentHeading);
  }

  for (int i = 0; i < num_boids; i++) {

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

  for (int i = 0; i < num_boids; i++) {
    rule1(boids, i, q);
    rule2(boids, i, q);
    rule3(boids, i, q);
    rule4(boids, i, q);
  }

  for (int i = 0; i < num_boids; i++) {

    float heading_weight = 1.0;
    float weights[] = {
        0.04,
        0.01,
        0.0025,
        0.0,
    };

    float new_x = heading_weight * cos(boids[i].currentHeading) +
                  weights[0] * cos(boids[i].headings[0]) +
                  weights[1] * cos(boids[i].headings[1]) +
                  weights[2] * cos(boids[i].headings[2]) +
                  weights[3] * cos(boids[i].headings[3]);

    float new_y = heading_weight * sin(boids[i].currentHeading) +
                  weights[0] * sin(boids[i].headings[0]) +
                  weights[1] * sin(boids[i].headings[1]) +
                  weights[2] * sin(boids[i].headings[2]) +
                  weights[3] * sin(boids[i].headings[3]);

    boids[i].currentHeading = atan2(new_y, new_x);

    boids[i].x += BOID_SPEED * cos(boids[i].currentHeading);
    boids[i].y += BOID_SPEED * sin(boids[i].currentHeading);
  }
}

int main(int argc, char *argv[]) {
  int num_boids = 0;
  struct Boid boids[MAX_BOIDS];

  float fps = 0;
  int frame = 0;
  int target_fps = 0;

  add_arg('c', "no-cap-framerate", "Start with a uncapped framerate.");
  add_arg('d', "debug", "Start with debug view enabled.");
  add_arg('f', "fps", "Target FPS (default 60).");
  add_arg('n', "num", "Number of boids in simulation (default 256).");
  add_arg('p', "pause", "Start paused.");
  add_arg('s', "seed", "Seed to use for random generation.");
  add_arg('y', "dynamic",
          "Number of boids dynamically changes based on framerate.");

  parse_opts(argc, argv);

  bool cap_framerate = !get_is_set('c');
  bool debug_view = get_is_set('d');
  bool paused = get_is_set('p');
  bool dynamic = get_is_set('y');

  if (get_value('f')) {
    target_fps = atoi(get_value('f'));
    cap_framerate = true;
  }

  if (!target_fps) {
    target_fps = 60;
  }

  int target_boids = 1000;
  if (get_is_set('n')) {
    target_boids = atoi(get_value('n'));

    if (target_boids < 0) {
      target_boids = 0;
    }

    if (target_boids > MAX_BOIDS) {
      target_boids = MAX_BOIDS;
    }
  }

  if (get_value('s')) {
    srand(atoi(get_value('s')));
  } else {
    srand(time(0));
  }

  num_boids = initialize_positions(boids, target_boids);

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();

  SDL_Window *window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                                        SDL_WINDOW_SHOWN);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  TTF_Font *font = TTF_OpenFont("res/LiberationSans-Regular.ttf", 12);

  SDL_Color white = {255, 255, 255};

  static int mouse_x = 0;
  static int mouse_y = 0;

  int clicked_x = -1;
  int clicked_y = -1;
  int clicked_button = -1;

  SDL_Event event;
  bool running = true;
  while (running) {
    Uint32 begin = SDL_GetTicks();

    while (SDL_PollEvent(&event)) {
      switch (event.type) {

      case SDL_QUIT:
        running = false;
        break;

      case SDL_MOUSEMOTION:
        mouse_x = event.motion.x;
        mouse_y = event.motion.y;
        break;

      case SDL_MOUSEBUTTONDOWN:
        clicked_x = event.button.x;
        clicked_y = event.button.y;
        clicked_button = event.button.button;
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
          // printf("Unhandled Key: %d\n", event.key.keysym.sym);
          break;
        }
        break;

      case SDL_POLLSENTINEL:
        break;

      default:
        // printf("Unhandled Event: %d\n", event.type);
        break;
      }
    }

    struct Quadtree q = {0};
    q.w = WIDTH;
    q.h = HEIGHT;

    for (int i = 0; i < target_boids; i++) {
      quadtree_insert(&q, i, boids[i].x, boids[i].y);
    }

    render(renderer, window, boids, num_boids, frame, fps, white, &q, font,
           debug_view);

    if (!paused) {
      simulate_boids(boids, num_boids, &q);
      frame++;
    }

    quadtree_free(&q);

    Uint32 end = SDL_GetTicks();
    if (cap_framerate) {
      int delay = 1000 / target_fps - (end - begin);
      if (delay > 0) {
        SDL_Delay(delay);
        if (dynamic) {
          add_boid(boids, &num_boids);
        }
      } else {
        if (dynamic) {
          if (frame % 100 == 0) {
            remove_boid(&num_boids);
          }
        }
      }
    }

    if (frame % 10 == 0) {
      fps = 1000.0 / (SDL_GetTicks() - begin);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
