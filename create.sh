#!/bin/bash

PREFIX=21.T12995
VERBOSE=--verbose

set -x
CREATE_HANDLE="./handle $VERBOSE --output last_handle.json --config config.json create "

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
