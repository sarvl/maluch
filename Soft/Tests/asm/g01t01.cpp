#include "../../Assembler/asm.h"

int code()
{
	//label backwards over instructions
	using namespace reg;

	label("loop");

	i_mov(R1, R2);
	i_mov(R1, 12);

	i_jmp("loop");

	return 0;
}
