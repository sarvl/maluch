#include "./asm_helper.h"

#include "./asm.h"

#include <cstdio> //printf

extern int code_position;

void save_regs(uint16_t const addr)
{
	for(int i = 0; i < 16; i++)
		i_stw(static_cast<t_reg>(i), addr + i);

	return;
}

void var_inc(uint16_t const addr, t_reg const reg)
{
	i_ldw(reg, addr);
	i_add(reg, 1);
	i_stw(reg, addr);

	return;
}
void var_dec(uint16_t const addr, t_reg const reg)
{
	i_ldw(reg, addr);
	i_sub(reg, 1);
	i_stw(reg, addr);

	return;
}

void interrupt_enable(int const n)
{
	if(n > 7)
	{
		printf("ERROR: interrupt ID cannot be greater than 7\n");
		return;
	}

	i_or(reg::R1, 0b1 << (15 - n)); //upper half of that register

	return;
}


void busy_wait()
{
	i_bss(code_position);
	return;
}

void mem_allocate(uint16_t const addr, std::string_view const data)
{
	auto const saved = code_position;

	code_position = addr;

	for(auto const c : data)
		word(static_cast<uint16_t>(c));
	
	code_position = saved;
	return;
}

