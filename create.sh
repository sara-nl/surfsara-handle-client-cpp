#!/bin/bash

PID_SERVER=https://epic5.storage.surfsara.nl/api/handles
PORT=8003
PREFIX=21.T12995
VERBOSE=--verbose

set -x
CREATE_HANDLE="./handle $VERBOSE --key 308_21.T12995_TRAINING_privkey.pem --cert 308_21.T12995_TRAINING_certificate_only.pem --insecure --port $PORT --url $PID_SERVER create $PREFIX --json "

$CREATE_HANDLE '{"values": [{
                             "index":1,
                             "type":"URL",
                             "data": { "format": "string", "value": "www.test1.com" }
                            },{
                             "index": 100,
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