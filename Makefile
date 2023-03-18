CC := clang
LIBS := -lSDL2 -lm -lSDL2_ttf
CFLAGS := -g -Wall -Werror -Wpedantic

all: build/boids

build/boids: ./src/main.c
	mkdir -p build/
	${CC} ${CFLAGS} ./src/main.c ${LIBS} -o $@

clean:
	rm -rf build/

.PHONY: all clean test example
