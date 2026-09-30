#!/bin/sh -e

find -name '*.cpp' -o -name '*.h' | xargs clang-format -i -style=file:./.clang-format
