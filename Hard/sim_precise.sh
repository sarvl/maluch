#!/bin/bash

line="-------------------------------------------------------------------------"
testbench=false

while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      echo "Usage: $0 [-h] [module names]"
      shift # past argument
      exit 0
      ;;
    -t|--testbench)
      testbench=true
      shift
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      POSITIONAL_ARGS+=("$1") # save positional arg
      shift # past argument
      ;;
  esac
done

if [ ${#POSITIONAL_ARGS[@]} -eq 0 ]; then
    echo "[ERROR] At least one module expected"
    exit 1
fi

set -- "${POSITIONAL_ARGS[@]}"

echo $line
printf "Startring preprocessing of top module ...\n"

$testbench && \
iverilog -g2012 tb/$1.sv -E -Irtl -o build/oo.sv -DPRECISE_SIM \
|| \
 iverilog -g2012 rtl/$1.sv -E -Irtl -o build/oo.sv -DPRECISE_SIM

if [ $? -eq 0 ]; then
    printf "\b\b\b\b Succes\n"
else
    printf "\b\b\b\b Failed!\n"
    exit 1
fi

printf "Converting SystemVerilog modules to Verilog ...\n" 

sv2v build/oo.sv -w build/oo.v

if [ $? -eq 0 ]; then
    printf "\b\b\b\b Succes\n"
else
    printf "\b\b\b\b Failed!\n"
    exit 1
fi

printf "Creating simulation files ...\n" 

iverilog -g2012 build/oo.v -s $1 -o build/oo.vvp

if [ $? -eq 0 ]; then
    printf "\b\b\b\b Succes\n"
else
    printf "\b\b\b\b Failed!\n"
    exit 1
fi

printf "Running simulation ...\n" 

vvp build/oo.vvp -fst -dumpfile=build/oo

if [ $? -eq 0 ]; then
    printf "\b\b\b\b Succes\n"
else
    printf "\b\b\b\b Failed!\n"
    exit 1
fi

