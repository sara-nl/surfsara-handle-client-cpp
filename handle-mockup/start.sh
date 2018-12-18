#!/bin/bash

PORT=5000
PID_FILE=/var/handle_mock_${PORT}.pid
LOG_FILE=/var/log/handle_mock_${PORT}.log
DIRNAME=$( dirname "$(readlink -f "$0")" )

if [ -e $PID_FILE ]
then
    echo "handle process pid file exists: "$PID_FILE
    echo "pid: "$( cat $PID_FILE)
    exit 8
fi

$DIRNAME/handle_mock.py --port $PORT --pid_file $PID_FILE > $LOG_FILE &
echo "log file: $LOG_FILE"


