#!/bin/bash

VERBOSE=--verbose
TYPE=$1
VALUE=$2

set -x
./handle --config config.json iset /path/to/testfile.txt $TYPE $VALUE

