#!/bin/bash

PREFIX=21.T12995
USER=21.T12995
PASSWORD=$( cat reverse_lookup_password.txt )
PORT=8003
URL=https://epic5.storage.surfsara.nl/hrls/handles
VERBOSE=--verbose
set -x



./handle --lookup_url $URL \
         --lookup_port $PORT \
         --lookup_user $USER \
         --lookup_password $PASSWORD \
         --lookup_insecure \
         ${VERBOSE} \
         lookup \
         ${PREFIX} \
         'URL=*'
