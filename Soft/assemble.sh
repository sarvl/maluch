#!/bin/bash



if [ "$#" -lt 1 ]; then
	echo "assemble.sh requires at least 1 argument - the name of the file to assemble"
	exit 1
fi

if [ "$#" -gt 3 ]; then
	echo "assemble.sh requires at most 2 argument - the name of the file to assemble and the output file"
	exit 1
fi
set -e 

make --no-print-directory -C Assembler/ assemble FILE="$1" OPT="-O3"

if [ "$#" -eq 2 ]; then
	./Assembler/main > "$2"
else
	./Assembler/main
fi
