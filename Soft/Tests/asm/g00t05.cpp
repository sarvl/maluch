#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_xor(R5, R10);
	i_xor(R5, 12);


	return 0;
}
