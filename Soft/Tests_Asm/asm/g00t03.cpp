#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_and(R5, R10);
	i_and(R5, 12);


	return 0;
}
