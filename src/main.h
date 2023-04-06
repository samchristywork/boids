#ifndef MAIN_H
#define MAIN_H

enum {
  WIDGET_SLIDER,
  WIDGET_CHECKBOX,
};

struct Boid {
  float x;
  float y;
  float currentHeading;
  float headings[4];
};

struct Widget {
  char name[50];
  float min;
  float max;
  float value_f;
  float value_b;
  float minx;
  float miny;
  float width;
  float height;
  int type;
};

#endif
