CC := gcc
LIBS := -lSDL2 -lm -lSDL2_gfx -lSDL2_ttf
CFLAGS := -g -Wall -Werror -Wpedantic -O2

all: build/main

build/quadtree.o: src/quadtree.*
	mkdir -p build/
	${CC} ${CFLAGS} -c ./src/quadtree.c -o $@ ${CFLAGS}

build/main.o: src/main.c build/quadtree.o
	mkdir -p build/
	${CC} ${CFLAGS} -c ./src/main.c -o $@ ${CFLAGS}

build/main: build/main.o build/quadtree.o
	${CC} build/*.o ${LIBS} -o $@

.PHONY: coverage
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage -g
coverage: LIBS+=-lgcov --coverage
coverage: all

clean:
	rm -rf build/

.PHONY: all clean test example
