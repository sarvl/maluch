CD := '\033[0m'    #color default 
CR := '\033[0;31m' #color red
CY := '\033[0;33m' #color yellow 

FILES := out/log.o out/file.o

debug :=

SAN := $(if ${debug}, -g -fsanitize=undefined -fsanitize=address,                  )
OPT := $(if ${debug}, -Og                                       , -O3 -march=native)
STD := -std=c++20
WRN := -Wall -Wextra -pedantic
COMPILE_OPTIONS := ${SAN} ${OPT} ${STD} ${WRN}

all: ${FILES}

clean:
	@rm -frv out/*

remake: clean all
	
out/log.o: src/log.cpp src/log.h
	@echo -e ${CY}compiling utility/log.cpp${CD}
	@g++  ${COMPILE_OPTIONS} -c src/log.cpp  -o out/log.o 
out/file.o: src/file.cpp src/file.h src/log.h
	@echo -e ${CY}compiling utility/file.cpp${CD}
	@g++  ${COMPILE_OPTIONS} -c src/file.cpp -o out/file.o 

src/%.h:
	@touch $@
