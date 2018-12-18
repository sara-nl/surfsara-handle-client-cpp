#!/bin/bash

PORT=5000
PID_FILE=/var/handle_mock_${PORT}.pid


if [ ! -e $PID_FILE ]
then
    echo "handle process pid file does not exists: "$PID_FILE
    exit 8
fi

kill $( cat $PID_FILE )



