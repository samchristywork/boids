#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Argument {
  char *long_name;
  char *description;
  char *value;
  int set;
  void (*function)();
};

struct Argument Arguments[26] = {{0}};

void set_arg_function(void (*function)(), char short_name) {
  int idx = short_name - 'a';

  if (idx >= 0 && idx < 26) {
    Arguments[idx].function = function;
  }
}

void add_arg(char short_name, char *long_name, char *description) {
  int idx = short_name - 'a';

  if (idx >= 0 && idx < 26) {
    Arguments[idx].long_name = (char *)malloc(strlen(long_name) + 1);
    Arguments[idx].description = (char *)malloc(strlen(description) + 1);

    strcpy(Arguments[idx].long_name, long_name);
    strcpy(Arguments[idx].description, description);
  }
}

int get_is_set(char short_name) {
  int idx = short_name - 'a';

  if (idx >= 0 && idx < 26) {
    return Arguments[idx].set;
  }

  return 0;
}

char *get_value(char short_name) {
  int idx = short_name - 'a';

  if (idx >= 0 && idx < 26) {
    return Arguments[idx].value;
  }

  return NULL;
}

void parse_opts(int argc, char *argv[]) {
}

#endif
