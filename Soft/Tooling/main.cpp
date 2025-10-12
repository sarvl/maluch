#include <cstdio> //printf

#include <cstdint>

#include "./assemble.h"
#include "./simulate.h"

int code_position = 0;
bool label_only = false;

int main()
{

#if defined(ASSEMBLE)

	//2 passes to 1. find labels, 2. put all relevant data
	
	code_position = 0;
	label_only = true;
	code();

	code_position = 0;
	label_only = false;
	code();

#elif defined(SIMULATE)

	//first pass only identifies labels

	code_position = 0;
	label_only = true;
	code();

	code_position = 0;
	label_only = false;
	code();


	print_regs();


#else

	#error "No option specified, use through assembly.sh and simulate.sh"

#endif 
}
