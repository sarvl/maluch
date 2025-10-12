#pragma once

#include "./asm.h"

#include <string_view>

#include <cstdint>

#define PROC_CREATE(name, proc)	\
	label(name);\
	proc; \
	i_ret();

#define IHT_CREATE(name, proc)	\
	label(name);\
	proc; \
	i_iret();

void save_regs(uint16_t const addr);

extern int code_position;

template<typename T, size_t N>
void mem_allocate(uint16_t const addr, T const (&arr)[N])
{
	auto const saved = code_position;

	code_position = addr;

	for(size_t i = 0; i < N; i++)
		word(static_cast<uint16_t>(arr[i]));
	
	code_position = saved;
	return;
}
template<typename T>
void mem_allocate(uint16_t const addr, T const val)
{
	auto const saved = code_position;

	code_position = addr;

	word(static_cast<uint16_t>(val));
	
	code_position = saved;
	return;
}

void mem_allocate(uint16_t const addr, std::string_view const data);

void var_inc(uint16_t const addr, t_reg const reg = reg::R8);
void var_dec(uint16_t const addr, t_reg const reg = reg::R8);

void interrupt_enable(int const n);
void busy_wait();

template<typename T>
void set_iht(int const n, T const addr)
{
	int const temp = code_position;
	code_position = 0xFFF0 + n * 2;
	i_jmp(addr);
	code_position = temp;
	return;
}
