#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_stw(R5, R10);
	i_stw(R5, 12);


	return 0;
}
