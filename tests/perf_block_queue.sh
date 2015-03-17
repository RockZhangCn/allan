#!/bin/bash

EXE_PATH=../build/debug/bin/test_block_queue
if [ $# == 2 ]; then
    $EXE_PATH=$1 
fi

echo "program:" $EXE_PATH

time $EXE_PATH 1 1 10000000
time $EXE_PATH 1 2 10000000
time $EXE_PATH 1 3 10000000
