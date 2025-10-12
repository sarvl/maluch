#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "simulate.sh requires exactly 1 argument - the name of the file to simulate"
	exit 1
fi

make --no-print-directory -C Tooling/ simulate FILE="$1"
./Tooling/main
