CC := gcc
LIBS := -lSDL2 -lm -lSDL2_gfx -lSDL2_ttf
CFLAGS := -I src/

.PHONY: all
all: build/main

.PHONY: objects
objects: $(patsubst src/%.c, build/%.o, $(wildcard src/*.c))

build/%.o: src/%.c
	mkdir -p build
	$(CC) -c $(CFLAGS) $< -o $@

build/main: build/main.o build/quadtree.o build/render.o
	${CC} build/*.o ${LIBS} -o $@

.PHONY: run
run:
	make && ./build/main

.PHONY: watch
watch:
	make clean
	find src/ | entr -s 'pkill someuniquename; make && ln -sf ./main ./build/someuniquename && ./build/someuniquename &'

.PHONY: flamegraph
flamegraph:
	perf record --call-graph dwarf build/main && flamegraph --perfdata perf.data

.PHONY: performance
performance:
	make && ln -sf ./main ./build/someuniquename && build/someuniquename > /dev/null &
	watch -e -n0 cat /proc/$$(pgrep someuniquename)/status

.PHONY: list
list:
	@echo Valid Targets:
	@make -qp | awk -F':' '/^[a-zA-Z0-9][^$$#\/\t=]*:([^=]|$$)/ {split($$1,A,/ /);for(i in A)print A[i]}' | sort -u | grep -v "^Makefile$$" | sed 's/^/  - /g'

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
