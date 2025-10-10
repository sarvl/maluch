init:
	@make -C Simulator init
	@make -C Simulator default
	@ln -f -s Simulator/simulate ./
	
	@make -C Assembler init

sim:
	@make -C Simulator default
