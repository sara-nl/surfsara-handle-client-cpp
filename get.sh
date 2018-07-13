#!/bin/bash

PID_SERVER=https://epic5.storage.surfsara.nl/api/handles
PORT=8003
PREFIX=21.T12995
VERBOSE=--verbose
HANDLE=$1
if [ -z "$HANDLE" ]
then
    HANDLE=$( cat default_handle.txt )
fi

set -x
./handle $VERBOSE --key 308_21.T12995_TRAINING_privkey.pem --cert 308_21.T12995_TRAINING_certificate_only.pem --insecure --port $PORT --url $PID_SERVER get $HANDLE 

