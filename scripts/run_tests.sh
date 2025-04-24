#!/usr/bin/env bash

set -ev

cmake -S . -B build --fresh -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# C++ tests
./build/bin/test_monitor

# Python tests
python3 -m pytest
