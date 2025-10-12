#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_call(R10);
	i_call(12);


	return 0;
}
