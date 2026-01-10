#!/bin/bash

# check for input file
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <source_file.cpp> [input_file]"
    exit 1
fi

SRC="$1"
INPUT="$2" # optional

OUT="output"
g++ -std=c++20 "$SRC" utils.cpp -o "$OUT" -lcurl

if [ $? -ne 0 ]; then
    echo "compilation failed :("
    exit 1
fi

if [ -z "$INPUT" ]; then
    ./"$OUT"
else
    ./"$OUT" < "$INPUT"
fi
