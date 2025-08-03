#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_lsl(R5, R10);
	i_lsl(R5, 12);


	return 0;
}
