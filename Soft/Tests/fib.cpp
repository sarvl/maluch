#include "./Assembler/asm.h"

#include <string_view>

using namespace std::literals;
using namespace reg;

extern int code_position;

void print(char const c)
{
	i_mov(R5, c);
	i_stw(R5, 0);
	return;
}

void print(std::string_view const sv)
{
	for(auto const c : sv)
		print(c);

	return;
}

void print_hexd(t_reg const reg)
{
	i_and(reg, 0xF);
	i_cmp(reg, 10);
	i_bge(code_position + 8);

	i_add(reg, '0');
	i_stw(reg, 0);

	i_jmp(code_position + 6);

	//jump dest
	i_add(reg, 'A' - 10);
	i_stw(reg, 0);

	return;
}

void print_reg_hex(t_reg const reg)
{
	i_push(R5);
	print("0x"sv);

	i_mov(R5, reg);
	i_lsr(R5, 12);
	print_hexd(R5);

	i_mov(R5, reg);
	i_lsr(R5, 8);
	print_hexd(R5);

	i_mov(R5, reg);
	i_lsr(R5, 4);
	print_hexd(R5);

	i_mov(R5, reg);
	print_hexd(R5);

	print('\n');
	i_pull(R5);

	return;
}


int code()
{

	i_mov(R8,  0);
	i_mov(R9,  1);

	i_mov(R15, 12);
	i_bee("end");

	label("loop");
	print_reg_hex(R8);

	i_mov(R10, R8);
	i_mov(R8, R9);
	i_add(R9, R10);

	i_sub(R15, 1);
	i_bne("loop");

	label("end");

	return 0;
}
