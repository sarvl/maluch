#include "../Tooling/asm.h"

int code()
{
	using namespace reg;

	i_nand(R0, R1);
	i_nand(R0, 3);

	return 0;
}
