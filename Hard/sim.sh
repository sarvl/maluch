#!/bin/bash

line="-------------------------------------------------------------------------"

POSITIONAL_ARGS=()
RUNS=1000
quiet=false

while [[ $# -gt 0 ]]; do
  case $1 in
    -h|--help)
      echo "Usage: $0 [-h] [-q | --quiet] [-r | --runs <numb of runs>] [module names]"
      shift # past argument
      exit 0
      ;;
    -q|--quiet)
      quiet=true
      shift # past argument
      ;;
    -r|--runs-number)
      RUNS=$2
      shift
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
printf "Startring compilation of modules\n  Number of modules: %d\n" ${#POSITIONAL_ARGS[@]}

for module in "$@"; do
    printf "Compiling %s.sv under %s_tb.cpp: ..." $module $module

    ! $quiet && \
    verilator -cc rtl/${module}.sv --exe tb/${module}_tb.cpp tb/src/progmem.cpp \
    -Irtl --build -Wwarn-lint \
    --trace-fst --trace-structs --trace-params --trace-max-array 2048 --trace-max-width 1024 --trace-underscore \
    --timescale 1us/1ns --timing \
    --Mdir obj_sim_${module} \
    -o ${module}-run \
    -CFLAGS -Itb/src \
    || \
    verilator -cc rtl/${module}.sv --exe tb/${module}_tb.cpp tb/src/progmem.cpp \
    -Irtl --build -Wwarn-lint \
    --trace-fst --trace-structs --trace-params --trace-max-array 2048 --trace-max-width 1024 --trace-underscore \
    --timescale 1us/1ns --timing \
    --Mdir obj_sim_${module} \
    -o ${module}-run > /dev/null \
    -CFLAGS -Itb/src

    if [ $? -eq 0 ]; then
        printf "\b\b\b\b Succes\n"
    else
        printf "\b\b\b\b Failed!\n"
        exit 1
    fi
done

echo $line
printf "Startring simulation\n  Number of modules: %d\n  Runs: %d\n" ${#POSITIONAL_ARGS[@]} $RUNS

for module in "$@"; do
    printf "\nSimulating %s.sv under %s_tb.cpp\n" $module $module

    ! $quiet && ./obj_sim_${module}/${module}-run n=$RUNS || ./obj_sim_${module}/${module}-run n=$RUNS > /dev/null

    if [ $? -eq 0 ]; then
        printf "Simulation succesfull\n"
    else
        printf "Simulation failed!\n"
        exit 1
    fi
    echo ""
    echo "########"
done

exit 0