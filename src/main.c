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

float g_fps = 0;
int g_num_boids = 0;
struct Boid g_boids[MAX_BOIDS];

float random_float(float low, float high) {
  return low + (high - low) * (float)rand() / (float)RAND_MAX;
}

void add_boid() {
  if (g_num_boids < MAX_BOIDS) {
    g_boids[g_num_boids].x = random_float(0, WIDTH);
    g_boids[g_num_boids].y = random_float(0, HEIGHT);
    g_boids[g_num_boids].current_heading = random_float(0, 3.141 * 2);
    g_num_boids++;
  }
}

void remove_boid() { g_num_boids--; }

void initialize_positions(int n) {
  for (int i = 0; i < n; i++) {
    add_boid();
  }
}

float boid_dist_2(int a, int b) {
  float dx = g_boids[a].x - g_boids[b].x;
  float dy = g_boids[a].y - g_boids[b].y;

  return dx * dx + dy * dy;
}

float boid_dist(int a, int b) { return sqrt(boid_dist_2(a, b)); }

// separation: steer to avoid crowding local flockmates
void rule1(int idx, struct Quadtree *q) {
  g_boids[idx].rule1_heading = g_boids[idx].current_heading;

  int length;
  int *nearby = quadtree_query(q, g_boids[idx].x - RADIUS_MIN / 2,
                               g_boids[idx].y - RADIUS_MIN / 2, RADIUS_MIN,
                               RADIUS_MIN, &length);

  for (int j = 0; j < length; j++) {
    int i=nearby[j];
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MIN) {
        float dx = g_boids[idx].x - g_boids[i].x;
        float dy = g_boids[idx].y - g_boids[i].y;
        g_boids[idx].rule1_heading = atan2(dy, dx);
      }
    }
  }

  free(nearby);
}

// alignment: steer towards the average heading of local flockmates
void rule2(int idx, struct Quadtree *q) {
  g_boids[idx].rule2_heading = g_boids[idx].current_heading;

  float sum_x_heading = 0;
  float sum_y_heading = 0;
  int n = 0;

  int length;
  int *nearby = quadtree_query(q, g_boids[idx].x - RADIUS_MAX / 2,
                               g_boids[idx].y - RADIUS_MAX / 2, RADIUS_MAX,
                               RADIUS_MAX, &length);

  for (int j = 0; j < length; j++) {
    int i=nearby[j];
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_heading += cos(g_boids[i].current_heading);
        sum_y_heading += sin(g_boids[i].current_heading);
        n++;
      }
    }
  }

  free(nearby);

  if (n != 0) {
    g_boids[idx].rule2_heading = atan2(sum_y_heading, sum_x_heading);
  }
}

// cohesion: steer to move towards the average position (center of mass) of
// local flockmates
void rule3(int idx, struct Quadtree *q) {
  g_boids[idx].rule3_heading = g_boids[idx].current_heading;

  float sum_x_mass = 0;
  float sum_y_mass = 0;
  int n = 0;

  int length;
  int *nearby = quadtree_query(q, g_boids[idx].x - RADIUS_MAX / 2,
                               g_boids[idx].y - RADIUS_MAX / 2, RADIUS_MAX,
                               RADIUS_MAX, &length);

  for (int j = 0; j < length; j++) {
    int i=nearby[j];
    if (i != idx) {
      float dist = boid_dist(idx, i);
      if (dist < RADIUS_MAX) {
        sum_x_mass += g_boids[i].x;
        sum_y_mass += g_boids[i].y;
        n++;
      }
    }
  }

  free(nearby);

  if (n != 0) {
    float dx = sum_x_mass / (float)n - g_boids[idx].x;
    float dy = sum_y_mass / (float)n - g_boids[idx].y;
    g_boids[idx].rule3_heading = atan2(dy, dx);
  }
}

// noise: steer in random directions
void rule4(int idx, struct Quadtree *q) {
  g_boids[idx].rule4_heading = g_boids[idx].current_heading;

  g_boids[idx].rule4_heading += random_float(-0.1, 0.1);
}


void simulate_boids(struct Quadtree *q) {

  for (int i = 0; i < g_num_boids; i++) {
    g_boids[i].x += BOID_SPEED * cos(g_boids[i].current_heading);
    g_boids[i].y += BOID_SPEED * sin(g_boids[i].current_heading);
  }

  for (int i = 0; i < g_num_boids; i++) {

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

  for (int i = 0; i < g_num_boids; i++) {
    rule1(i, q);
    rule2(i, q);
    rule3(i, q);
    rule4(i, q);
  }

  for (int i = 0; i < g_num_boids; i++) {

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

int main(int argc, char *argv[]) {
  int frame = 0;
  int target_fps = 60;

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

  int num_boids = 256;
  if (get_value('n')) {
    num_boids = atoi(get_value('n'));

    if (num_boids < 0) {
      num_boids = 0;
    }

    if (num_boids > MAX_BOIDS) {
      num_boids = MAX_BOIDS;
    }
  }

  if (get_value('s')) {
    srand(atoi(get_value('s')));
  } else {
    srand(time(0));
  }

  initialize_positions(num_boids);

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

    for (int i = 0; i < g_num_boids; i++) {
      quadtree_insert(&q, i, g_boids[i].x, g_boids[i].y);
    }

    draw_boids(renderer, g_boids, g_num_boids, debug_view, &q);

    if (!paused) {
      simulate_boids(&q);
      frame++;
    }

    quadtree_free(&q);

    char frame_text[256];
    snprintf(frame_text, 255, "Frame: %d", frame);
    draw_text(renderer, font, 0, 0, white, frame_text);

    char framerate_text[256];
    snprintf(framerate_text, 255, "FPS: %d", (int)g_fps);
    draw_text(renderer, font, 0, 16, white, framerate_text);

    char num_boids_text[256];
    snprintf(num_boids_text, 255, "Boids: %d", (int)g_num_boids);
    draw_text(renderer, font, 0, 32, white, num_boids_text);

    SDL_RenderPresent(renderer);

    Uint32 end = SDL_GetTicks();
    if (cap_framerate) {
      int delay = 1000 / target_fps - (end - begin);
      if (delay > 0) {
        SDL_Delay(delay);
        if (dynamic) {
          add_boid();
        }
      } else {
        if (dynamic) {
          if (frame % 100 == 0) {
            remove_boid();
          }
        }
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
