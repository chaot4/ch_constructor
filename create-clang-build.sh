#!/bin/bash

set -e

self="$(readlink -f "$0")"
cd "$(dirname "${self}")"

mkdir -p clang-build
cd clang-build
rm -f CMakeCache.txt
cmake -D CMAKE_CXX_COMPILER="$(which clang++)" -D CMAKE_CXX_FLAGS="-fsanitize=integer,thread" ..
make -j
ctest -V

