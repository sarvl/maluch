default: out/main.o out/instruction.o out/file.o out/tui.o ../utility/out/log.o ../utility/out/file.o
	@echo linking
	@g++ -std=c++23 -O3 -Wall -Wextra -pedantic out/* ../utility/out/log.o ../utility/out/file.o -lncurses -o simulate
	
init:
	@mkdir -p out/

out/main.o: src/main.cpp
	@echo compiling main.cpp
	@g++ -std=c++23 -O3 -Wall -Wextra -pedantic src/main.cpp -lncurses -c -o out/main.o

out/instruction.o: src/instruction.cpp
	@echo compiling instruction.cpp
	@g++ -std=c++23 -O3 -Wall -Wextra -pedantic src/instruction.cpp -lncurses -c -o out/instruction.o

out/file.o: src/file.cpp
	@echo compiling file.cpp
	@g++ -std=c++23 -O3 -Wall -Wextra -pedantic src/file.cpp -lncurses -c -o out/file.o

out/tui.o: src/tui.cpp
	@echo compiling tui.cpp
	@g++ -std=c++23 -O3 -Wall -Wextra -pedantic src/tui.cpp -lncurses -c -o out/tui.o

src/main.cpp:
	@touch src/main.cpp

src/instruction.cpp:
	@touch src/instruction.cpp

src/file.cpp:
	@touch src/file.cpp

src/tui.cpp:
	@touch src/tui.cpp
