#!/bin/sh

# Exit if any commands fail
set -e

cmake ..
cmake --build . --parallel 2
