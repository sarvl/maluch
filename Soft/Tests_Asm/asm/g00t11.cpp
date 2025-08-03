#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_jmp(R10);
	i_jmp(12);


	return 0;
}
