#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_lsr(R5, R10);
	i_lsr(R5, 12);


	return 0;
}
