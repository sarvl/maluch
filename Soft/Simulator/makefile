default: out/main.o 
	@echo linking 
	@g++ --std=c++26 -DASSEMBLE=1 -Wall -Wextra -pedantic ../utility/out/log.o ../utility/out/file.o out/main.o -o simulate -lncurses

out/main.o: main.cpp
	@echo compiling main.cpp	
	@g++ --std=c++26 -DASSEMBLE=1 ${OPT} -Wall -Wextra -pedantic -c -o out/main.o main.cpp 

main.cpp:
	@touch main.cpp
