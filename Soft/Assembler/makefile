assemble: out/main.o out/assemble.o out/asm_helper.o
	@echo assembling ${FILE}
	@g++ -std=c++23 -DASSEMBLE=1 -Wall -Wextra -pedantic out/* ../${FILE} -o main ${OPT}

init:
	@mkdir -p out

out/main.o: main.cpp
	@g++ -std=c++23 -DASSEMBLE=1 ${OPT} -Wall -Wextra -pedantic -c -o out/main.o main.cpp 

out/assemble.o: assemble.cpp
	@g++ -std=c++23 -DASSEMBLE=1 ${OPT} -Wall -Wextra -pedantic -c -o out/assemble.o assemble.cpp 

out/asm_helper.o: asm_helper.cpp
	@g++ -std=c++23 -DASSEMBLE=1 ${OPT} -Wall -Wextra -pedantic -c -o out/asm_helper.o asm_helper.cpp 

main.cpp:
	@touch main.cpp

assemble.cpp:
	@touch assemble.cpp

asm_helper.cpp:
	@touch asm_helper.cpp
