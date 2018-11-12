#!/bin/bash

VERBOSE=--verbose
TYPE=$1


set -x
if [ -z "$TYPE" ]
then
    ./handle $VERBOSE --config config.json iget /path/to/testfile3.txt
else
    ./handle --config config.json iget /path/to/testfile3.txt $TYPE
fi
