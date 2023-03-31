#!/bin/sh

cmake --build build
./bin/cpm "$@"
