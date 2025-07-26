default: out/main.o
	@echo linking
	@g++ -O3 -Wall -Wextra -pedantic out/* -lncurses -o main
	

out/main.o: src/main.cpp
	@echo compiling main.cpp
	@g++ -O3 -Wall -Wextra -pedantic src/main.cpp -lncurses -c -o out/main.o

src/main.cpp:
	@touch src/main.cpp


