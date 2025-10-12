#include "../Tooling/asm.h"

int code()
{
	using namespace reg;

	i_mov(R8,  0);
	i_mov(R9,  1);
	i_mov(R10, 0);


	i_mov(R15, 4);
	i_cmp(R15, 0);
	i_bre("end");
	

	label("loop");

	i_mov(R10, R8);
	i_mov(R8, R9);
	i_add(R9, R10);

	i_sub(R15, 1);
	i_bre("end");

	label("end");

	return 0;
}
