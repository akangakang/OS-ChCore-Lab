#!/bin/bash

[ ! -d build ] && mkdir build

cd build
cmake .. -G Ninja
ninja && ctest && ninja lcov && echo -e "\n\nPlease open ./build/report/index.html for the coverage report"
