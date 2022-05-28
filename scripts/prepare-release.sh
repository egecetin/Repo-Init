#!/bin/bash

# Clear build directory
rm -rf ../build/*

# Compile Release
cd ../build
cmake -DCMAKE_BUILD_TYPE=Release -Wno-dev ..
cmake --build .

# Copy rpm packages to dist
# TODO <---------------------------------------------------------
