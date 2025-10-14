assemble:
	@echo assembling ${FILE}
	@g++ -DASSEMBLE=1 -Wall -Wextra -pedantic assemble.cpp main.cpp ../${FILE} -o main

simulate:
	@echo simulating ${FILE}
	@g++ -DSIMULATE=1 -Wall -Wextra -pedantic simulate.cpp main.cpp ../${FILE} -o main
