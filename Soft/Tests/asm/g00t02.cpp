#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_sub(R5, R10);
	i_sub(R5, 12);


	return 0;
}
