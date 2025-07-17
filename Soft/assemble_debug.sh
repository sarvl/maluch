#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "assemble.sh requires exactly 1 argument - the name of the file to assemble"
	exit 1
fi

set -e 

make --no-print-directory -C Assembler/ assemble FILE="$1" OPT="-fsanitize=undefined -fsanitize=address -Og"
./Assembler/main
