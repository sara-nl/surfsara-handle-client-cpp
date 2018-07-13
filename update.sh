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

UPDATE="./handle $VERBOSE --key 308_21.T12995_TRAINING_privkey.pem --cert 308_21.T12995_TRAINING_certificate_only.pem --insecure --port $PORT --url $PID_SERVER update $HANDLE --json "

$UPDATE '{"values": [{
                             "index":2,
                             "type":"URL",
                             "data": { "format": "string", "value": "www.test1.com" }
                      },{
                             "index": 101,
                             "type": "HS_ADMIN",
                             "data": {
                                 "format":"admin",
                                 "value":{
                                     "index":200,
                                     "handle":"0.NA/'$PREFIX'",
                                     "permissions":"011111110011",
                                     "format":"admin"
                                 }
                             }
                            }
                      ]}'
