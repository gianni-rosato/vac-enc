#!/bin/sh

# Exit if any commands fail
set -e

cmake --fresh ..
cmake --build . --parallel 2
