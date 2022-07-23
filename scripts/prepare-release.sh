#!/bin/bash

# Clear build directory
rm -rf build/*

# Compile Release
cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev -S . -B build
cmake --build build --target package --parallel
