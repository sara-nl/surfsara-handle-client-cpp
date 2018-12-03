#!/bin/bash

VERBOSE=--verbose

set -x
./handle --config config.json iset /path/to/testfile3.txt $@

