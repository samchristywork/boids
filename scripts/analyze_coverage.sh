#!/bin/bash

make clean || exit
make coverage || exit
./build/main || exit
gcov src/main.c -o build/* || exit
lcov --capture --directory build/ --output-file build/coverage.info || exit
genhtml build/coverage.info --output-directory build/ || exit
mkdir -p gcov || exit
mv *.gcov gcov/ || exit
firefox build/index.html
