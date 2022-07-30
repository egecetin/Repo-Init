#!/bin/bash

read -p "Enter new name: " replace

sed -i "s/XXX/${replace}/" CMakeLists.txt
