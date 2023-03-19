CC := clang
LIBS := -lSDL2 -lm -lSDL2_gfx -lSDL2_ttf
CFLAGS := -g -Wall -Werror -Wpedantic -O2

all: build/boids

build/quadtree.o: src/quadtree.*
	mkdir -p build/
	${CC} ${CFLAGS} -c ./src/quadtree.c -o $@

build/boids: src/main.c build/quadtree.o
	mkdir -p build/
	${CC} ${CFLAGS} build/quadtree.o ./src/main.c ${LIBS} -o $@

clean:
	rm -rf build/

.PHONY: all clean test example
