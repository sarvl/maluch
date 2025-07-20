#include "./Assembler/asm.h"

#include <cstdio>

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

void ihr_timer()
{
	i_push(R5);

	print("timer ");

	i_ldw(R8, 0x8000);
	print_reg_hex(R8);

	i_add(R8, 1);
	i_stw(R8, 0x8000);

	//clear the interrupt
	i_in(0, R5);

	i_pull(R5);
	i_iret();

	return;
}

void ihr_keyboard()
{
	i_push(R5);

	//clear the interrupt and read character
	i_in(1, R5);

	i_stw(R5, 0x0000);

	i_cmp(R5, '\n');
	i_bne(code_position + 6);

	i_mov(R5, '>');
	i_stw(R5, 0x0000);

	//jump dest
	i_pull(R5);
	i_iret();

	return;
}

void busy_wait()
{
	i_bss(code_position);
	return;
}

template<typename T>
void set_iht(int const n, T const addr)
{
	//set command to initialize 0th entry in IHT
	busy_wait();
	i_out(0, n);
	
	//send an address
	busy_wait();
	i_out(0, addr);

	return;
}

void interrupt_enable(int const n)
{
	if(n > 7)
	{
		printf("ERROR: interrupt ID cannot be greater than 7\n");
		return;
	}

	i_or(R1, 0b1 << (15 - n)); //upper half of that register

	return;
}

int code()
{
	i_jmp("start");

	label("IHR_0");
	ihr_timer();
	label("IHR_1");
	ihr_keyboard();


	label("start");
	set_iht(0, "IHR_0");
	//will cause invalid instruction err
	set_iht(1, "IHR_1");
	set_iht(2, 0x8123);
	set_iht(3, 0x8123);
	set_iht(4, 0x8123);
	set_iht(5, 0x8123);
	set_iht(6, 0x8123);
	set_iht(7, 0x8123);

	//make the screen kind of like a shell :D
	print(">");

	//enable specific keyboard interrupt
	interrupt_enable(1);

	//wait indefinitely
	label("loop");
	i_jmp("loop");

	return 0;
}
