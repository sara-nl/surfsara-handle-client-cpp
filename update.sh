#!/bin/bash

VERBOSE=--verbose
HANDLE=$1
if [ -z "$HANDLE" ]
then
    HANDLE=$( ./get_last_handle.py )
fi

set -x

UPDATE="./handle $VERBOSE --config config.json update $HANDLE "

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
