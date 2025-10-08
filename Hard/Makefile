SHELL=/bin/bash
module =

.PHONY: clean
clean:
	@rm -rf ./obj_sim_*
	@rm -rf ./obj_dir
	@rm -rf ./waveforms/*



elaborate:
ifdef module
	verilator -cc rtl/${module}.sv --exe tb/${module}_tb.cpp \
    -Irtl --build -Wwarn-lint \
    --trace-fst --trace-structs --trace-params --trace-max-array 2048 --trace-max-width 1024 --trace-underscore \
    --timescale 1us/1ns --timing \
    --Mdir obj_sim_${module} \
    -o ${module}-run \
    -CFLAGS -Itb/src -Dtb/src
else
	@echo "Module not provided"
endif

run:
ifdef module
	./obj_sim_${module}/${module}-run
else
	@echo "Module not provided"
endif