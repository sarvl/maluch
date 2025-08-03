#include "../../Assembler/asm.h"

int code()
{
	using namespace reg;

	i_ldw(R5, R10);
	i_ldw(R5, 12);


	return 0;
}
