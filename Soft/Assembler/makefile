assemble: out/main.o out/assemble.o
	@echo assembling ${FILE}
	@g++ -DASSEMBLE=1 -Wall -Wextra -pedantic out/assemble.o out/main.o ../${FILE} -o main ${OPT}

out/main.o: main.cpp
	@g++ -DASSEMBLE=1 ${OPT} -Wall -Wextra -pedantic -c -o out/main.o main.cpp 

out/assemble.o: assemble.cpp
	@g++ -DASSEMBLE=1 ${OPT} -Wall -Wextra -pedantic -c -o out/assemble.o assemble.cpp 

main.cpp:
	@touch main.cpp

assemble.cpp:
	@touch assemble.cpp
