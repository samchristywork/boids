#ifndef MAIN_H
#define MAIN_H

struct Boid {
  float x;
  float y;
  float currentHeading;
  float headings[4];
};

struct Widget {
  char name[100];
  float min;
  float max;
  float value;
  float minx;
  float miny;
  float width;
  float height;
};

#endif
