#include "../../Assembler/asm.h"

int code()
{
	//label forwards over instructions
	using namespace reg;


	i_jmp("loop");

	i_mov(R1, R2);
	i_mov(R1, 12);

	label("loop");

	return 0;
}
