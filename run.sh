#!/bin/sh

cmake --build build
./bin/lang "$@"
