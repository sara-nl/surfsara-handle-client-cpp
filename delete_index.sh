#!/bin/bash

VERBOSE=--verbose
HANDLE=$1
if [ -z "$HANDLE" ]
then
    HANDLE=$( ./get_last_handle.py )
fi

set -x
./handle $VERBOSE --config config.json delete $HANDLE 100

