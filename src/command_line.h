#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

struct Argument {
  char *long_name;
  char *description;
  char *value;
  int set;
};

struct Argument Arguments[26] = {{0}};

void add_arg(char short_name, char *long_name, char *description) {
}

int get_is_set(char short_name) {
}

char *get_value(char short_name) {
}

void parse_opts(int argc, char *argv[]) {
}

#endif
