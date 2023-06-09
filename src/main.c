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

struct ScreenSize {
  int width;
  int height;
} screen_size = {1200, 700};

float random_float(float low, float high) {
  return low + (high - low) * (float)rand() / (float)RAND_MAX;
}

void add_boid(struct Boid *boids, int *num_boids) {
  if (*num_boids < MAX_BOIDS) {
    boids[*num_boids].x = random_float(0, screen_size.width);
    boids[*num_boids].y = random_float(0, screen_size.height);
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

void simulate_boids(struct Boid *boids, int num_boids, struct Widget *widgets,
                    int num_widgets, struct Quadtree *q) {

  for (int i = 0; i < num_boids; i++) {
    boids[i].x += BOID_SPEED * cos(boids[i].currentHeading);
    boids[i].y += BOID_SPEED * sin(boids[i].currentHeading);
  }

  for (int i = 0; i < num_boids; i++) {

    if (boids[i].x > screen_size.width) {
      boids[i].x = 0;
    }
    if (boids[i].x < 0) {
      boids[i].x = screen_size.width;
    }
    if (boids[i].y > screen_size.height) {
      boids[i].y = 0;
    }
    if (boids[i].y < 0) {
      boids[i].y = screen_size.height;
    }
  }

  for (int i = 0; i < num_boids; i++) {
    rule1(boids, i, q);
    rule2(boids, i, q);
    rule3(boids, i, q);
    rule4(boids, i, q);
  }

  for (int i = 0; i < num_boids; i++) {

    float heading_weight = widgets[3].value_f;
    float weights[] = {
        widgets[2].value_f,
        widgets[1].value_f,
        widgets[0].value_f,
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

  int num_widgets = 6;
  struct Widget widgets[num_widgets];

  // Typical values:
  // Separation 0.04,
  // Alignment  0.01,
  // Cohesion   0.0025,
  widgets[0].min = 0.0;
  widgets[0].max = 0.005 * 4;
  widgets[0].value_f = 0.0025;
  widgets[0].type = WIDGET_SLIDER;
  snprintf(widgets[0].name, 100, "Cohesion");

  widgets[1].min = 0.0;
  widgets[1].max = 0.02 * 4;
  widgets[1].value_f = 0.01;
  widgets[1].type = WIDGET_SLIDER;
  snprintf(widgets[1].name, 100, "Alignment");

  widgets[2].min = 0.0;
  widgets[2].max = 0.08 * 4;
  widgets[2].value_f = 0.04;
  widgets[2].type = WIDGET_SLIDER;
  snprintf(widgets[2].name, 100, "Separation");

  widgets[3].min = 0.5;
  widgets[3].max = 4.0;
  widgets[3].value_f = 1.0;
  widgets[3].type = WIDGET_SLIDER;
  snprintf(widgets[3].name, 100, "Speed");

  widgets[4].value_b = false;
  widgets[4].type = WIDGET_CHECKBOX;
  snprintf(widgets[4].name, 100, "Paused");

  widgets[5].value_b = false;
  widgets[5].type = WIDGET_CHECKBOX;
  snprintf(widgets[5].name, 100, "Follow mode");

  float fps = 0;
  int frame = 0;
  int target_fps = 0;

  add_arg('c', "no-cap-framerate", "Start with a uncapped framerate.");
  add_arg('d', "debug", "Start with debug view enabled.");
  add_arg('f', "fps", "Target FPS (default 60).");
  add_arg('n', "num", "Number of boids in simulation (default 256).");
  add_arg('p', "pause", "Start paused.");
  add_arg('s', "seed", "Seed to use for random generation.");
  add_arg('u', "fullscreen", "Fullscreen mode.");
  add_arg('y', "dynamic",
          "Number of boids dynamically changes based on framerate.");

  parse_opts(argc, argv);

  bool cap_framerate = !get_is_set('c');
  bool debug_view = get_is_set('d');
  bool dynamic = get_is_set('y');
  bool fullscreen = get_is_set('u');
  bool paused = get_is_set('p');
  widgets[4].value_b = paused;

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

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  TTF_Init();

  screen_size.width = 1200;
  screen_size.height = 700;
  SDL_Window *window = SDL_CreateWindow("Boids", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, screen_size.width, screen_size.height,
                                        SDL_WINDOW_SHOWN);

  if (fullscreen) {
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
  }

  SDL_GetWindowSize(window, &screen_size.width, &screen_size.height);

  num_boids = initialize_positions(boids, target_boids);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  TTF_Font *font = TTF_OpenFont("res/LiberationSans-Regular.ttf", 12);

  SDL_Color white = {255, 255, 255};

  int shade = 0x07;
  SDL_SetRenderDrawColor(renderer, shade, shade, shade, 0xff);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  int widget_selected = -1;
  bool lmb_down = false;

  SDL_Event event;
  bool running = true;
  while (running) {

    static int mouse_x = 0;
    static int mouse_y = 0;

    int clicked_x = -1;
    int clicked_y = -1;

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

      case SDL_MOUSEBUTTONUP:
        if (event.button.button == 1) {
          widget_selected = -1;
          lmb_down = false;
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == 1) {
          clicked_x = event.button.x;
          clicked_y = event.button.y;
          widget_selected = -1;
          lmb_down = true;
        }
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
          widgets[4].value_b = paused;
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

    for (int i = 0; i < num_widgets; i++) {
      if (clicked_x > widgets[i].minx && clicked_y > widgets[i].miny &&
          clicked_x < widgets[i].minx + widgets[i].width &&
          clicked_y < widgets[i].miny + widgets[i].height && lmb_down) {
        widget_selected = i;
      }
    }

    if (widget_selected != -1) {
      if (widgets[widget_selected].type == WIDGET_SLIDER) {
        widgets[widget_selected].value_f =
            (float)(mouse_x - widgets[widget_selected].minx) /
                (float)widgets[widget_selected].width *
                (widgets[widget_selected].max - widgets[widget_selected].min) +
            widgets[widget_selected].min;
        if (widgets[widget_selected].value_f < widgets[widget_selected].min) {
          widgets[widget_selected].value_f = widgets[widget_selected].min;
        }
        if (widgets[widget_selected].value_f > widgets[widget_selected].max) {
          widgets[widget_selected].value_f = widgets[widget_selected].max;
        }
      } else if (widgets[widget_selected].type == WIDGET_CHECKBOX) {
        lmb_down = false;
        widgets[widget_selected].value_b = !widgets[widget_selected].value_b;
        widget_selected = -1;
      }
    }

    if (widgets[4].value_b != paused) {
      paused = !paused;
    }

    struct Quadtree q = {0};
    q.w = screen_size.width;
    q.h = screen_size.height;

    for (int i = 0; i < target_boids; i++) {
      quadtree_insert(&q, i, boids[i].x, boids[i].y);
    }

    struct Context parent;
    parent.x = 0;
    parent.y = 0;
    parent.w = screen_size.width;
    parent.h = screen_size.height;

    struct Context child;
    child.x = 0;
    child.y = 0;
    child.w = screen_size.width;
    child.h = screen_size.height;

    if (widgets[5].value_b && num_boids > 0) {
      child.x = -boids[0].x + screen_size.width / 8;
      child.y = -boids[0].y + screen_size.height / 8;
      child.w = screen_size.width / 4;
      child.h = screen_size.height / 4;
    } else if (lmb_down && widget_selected == -1) {
      child.x = -mouse_x + screen_size.width / 8;
      child.y = -mouse_y + screen_size.height / 8;
      child.w = screen_size.width / 4;
      child.h = screen_size.height / 4;
    }

    render(renderer, window, boids, num_boids, widgets, num_widgets, parent,
           child, frame, fps, white, &q, font, debug_view);

    if (!paused) {
      simulate_boids(boids, num_boids, widgets, num_widgets, &q);
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
