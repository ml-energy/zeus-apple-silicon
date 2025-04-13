#!/usr/bin/env bash

set -ev

cmake -S . -B build --fresh
cmake --build build
./build/bin/test_monitor
