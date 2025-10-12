#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "assemble.sh requires exactly 1 argument - the name of the file to assemble"
	exit 1
fi

set -e 

make --no-print-directory -C Tooling/ assemble FILE="$1"
./Tooling/main
