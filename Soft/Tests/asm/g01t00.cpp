#include "../../Assembler/asm.h"

int code()
{
	//label backwards
	using namespace reg;

	label("loop");

 	i_jmp("loop");

	return 0;
}
