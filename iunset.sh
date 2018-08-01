#!/bin/bash

VERBOSE=--verbose
TYPE=$1


set -x
./handle --config config.json iunset /path/to/testfile.txt $TYPE

