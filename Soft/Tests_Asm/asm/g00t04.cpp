#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_or(R5, R10);
	i_or(R5, 12);


	return 0;
}
