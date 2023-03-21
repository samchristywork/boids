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

void usage() {
  printf("Usage:\n");
  for (int i = 0; i < 26; i++) {
    if (Arguments[i].description) {
      printf("  -%c,--%s ", 'a' + i, Arguments[i].long_name);
      for (int j = strlen(Arguments[i].long_name); j < 17; j++) {
        printf(" ");
      }
      printf("%s\n", Arguments[i].description);
    }
  }

  exit(EXIT_SUCCESS);
}

void parse_opts(int argc, char *argv[]) {
  add_arg('h', "help", "Display Usage statement.");
  set_arg_function(usage, 'h');

  int last_arg = -1;

  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      if (argv[i][0] == '-') {

        // Short opts
        if (argv[i][1] != '-') {
          for (int j = 1; j < strlen(argv[i]); j++) {
            last_arg = argv[i][j] - 'a';
            Arguments[last_arg].set = 1;
            if (Arguments[last_arg].function) {
              Arguments[last_arg].function();
            }
          }

          // Long opts
        } else {
          if (strlen(argv[i]) >= 3) {
            for (int j = 0; j < 26; j++) {
              if (Arguments[j].long_name) {
                if (strcmp(Arguments[j].long_name, argv[i] + 2) == 0) {
                  last_arg = j;
                  Arguments[last_arg].set = 1;
                  if (Arguments[last_arg].function) {
                    Arguments[last_arg].function();
                  }
                  break;
                }
              }
            }

            // Double hyphen
          } else {
            return;
          }
        }

        // Value
      } else {
        if (last_arg != -1) {
          Arguments[last_arg].value = (char *)malloc(strlen(argv[i]) + 1);
          strcpy(Arguments[last_arg].value, argv[i]);
        }
      }

      // Single character arg
    } else {
      if (last_arg != -1) {
        Arguments[last_arg].value = (char *)malloc(strlen(argv[i]) + 1);
        strcpy(Arguments[last_arg].value, argv[i]);
      }
    }
  }
}

#endif
