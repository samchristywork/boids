CC := gcc
LIBS := -lSDL2 -lm -lSDL2_gfx -lSDL2_ttf
CFLAGS := -I src/

.PHONY: all
all: build/main

build/quadtree.o: src/quadtree.*
	mkdir -p build/
	${CC} ${CFLAGS} -c ./src/quadtree.c -o $@ ${CFLAGS}

build/main.o: src/main.c build/quadtree.o
	mkdir -p build/
	${CC} ${CFLAGS} -c ./src/main.c -o $@ ${CFLAGS}

build/main: build/main.o build/quadtree.o
	${CC} build/*.o ${LIBS} -o $@

.PHONY: run
run:
	make && ./build/main

.PHONY: watch
watch:
	make clean
	find src/ | entr -s 'make && ./build/main'

.PHONY: flamegraph
flamegraph:
	perf record --call-graph dwarf build/main && flamegraph --perfdata perf.data

.PHONY: covhtml
covhtml:
	make clean || exit
	make coverage || exit
	./build/main || exit
	gcov src/main.c -o build/* || exit
	lcov --capture --directory build/ --output-file build/coverage.info || exit
	genhtml build/coverage.info --output-directory build/ || exit
	mkdir -p gcov || exit
	mv *.gcov gcov/ || exit
	firefox build/index.html

.PHONY: coverage
coverage: CFLAGS+=-fprofile-arcs -ftest-coverage -g
coverage: LIBS+=-lgcov --coverage
coverage: all

.PHONY: debug
debug: CFLAGS+=-g -Wall -Werror -Wpedantic
debug: LIBS+=
debug: all

.PHONY: release
release: CFLAGS+=-O2
release: LIBS+=
release: all

.PHONY: clean
clean:
	rm -rf build/
