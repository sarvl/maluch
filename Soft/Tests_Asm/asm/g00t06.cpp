#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_not(R5, R10);
	i_not(R5, 12);


	return 0;
}
