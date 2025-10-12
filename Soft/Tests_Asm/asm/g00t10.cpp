#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_test(R5, R10);
	i_test(R5, 12);


	return 0;
}
