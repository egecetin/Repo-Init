#!/bin/bash

read -p "Enter new name: " replace

sed -i "s/XXX/${replace}/" CMakeLists.txt
sed -i "s/XXX/${replace}/" tests/CMakeLists.txt
sed -i "s/XXX/${replace}/" tests/gtest_main.cpp
sed -i "s/XXX/${replace}/" tests/test-static-definitions.h.in
