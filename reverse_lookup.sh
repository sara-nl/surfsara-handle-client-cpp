#!/bin/bash
VERBOSE=--verbose
PREFIX=21.T12995
set -x

./handle --config config.json --handle_prefix $PREFIX ${VERBOSE} lookup 'URL=*'
