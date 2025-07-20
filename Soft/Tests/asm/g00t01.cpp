#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_add(R5, R10);
	i_add(R5, 12);


	return 0;
}
