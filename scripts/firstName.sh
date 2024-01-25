#!/bin/bash

while getopts :n: option
do
    case $option in
        n) ARGUMENT_NAME="$OPTARG" ;;
    esac
done

[ -z "${ARGUMENT_NAME}" ] && echo "$0: Missing argument: [-n name]" >&2
[ -z "${ARGUMENT_NAME}" ] && exit 1

sed -i "s/XXX/${ARGUMENT_NAME}/" CMakeLists.txt
sed -i "s/XXX/${ARGUMENT_NAME}/" CMakeLists.txt
sed -i "s/XXX/${ARGUMENT_NAME}/" tests/CMakeLists.txt
sed -i "s/XXX/${ARGUMENT_NAME}/" tests/unittests/gtest_main.cpp
sed -i "s/XXX/${ARGUMENT_NAME}/" tests/test-static-definitions.h.in
