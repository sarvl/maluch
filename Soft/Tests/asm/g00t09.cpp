#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_cmp(R5, R10);
	i_cmp(R5, 12);


	return 0;
}
