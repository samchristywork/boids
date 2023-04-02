#ifndef MAIN_H
#define MAIN_H

struct Boid {
  float x;
  float y;
  float currentHeading;
  float headings[4];
};

struct Widget {
  float min;
  float max;
  float value;
};

#endif
