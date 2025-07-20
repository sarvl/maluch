#include <cstdio> //printf

#include <cstdint>

#include "./assemble.h"

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

	for(auto const i : instructions)
		printf("%04X\n", i);

#else

	#error "No option specified, use through assembly.sh"

#endif 
}
