#include "../../Assembler/asm.h"

int code()
{
	//label forwards
	using namespace reg;

 	i_jmp("loop");

	label("loop");

	return 0;
}
