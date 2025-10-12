#include <ncurses.h>

#include <cstdint>

#include <bit> //countl_zero

#include "../../utility/src/log.h"

uint16_t memory[1 << 16];
uint16_t memory_monitor[1 << 16];
uint16_t registers[16];
uint16_t ip;
uint16_t int_mask_saved;
uint16_t flags_saved;
bool flag_sign;
bool flag_overflow;
bool flag_carry;
bool flag_zero;
int memory_address_accessed = -1;

WINDOW* output_window;

extern int output_win_cols, output_win_rows;

constexpr int hexd_to_val(char const hexd)
{
	constexpr signed char translate[128] = {
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	return translate[hexd & 0x7F];
}

uint16_t mem_read(uint16_t const addr)
{
	if(memory_monitor[addr])
		memory_address_accessed = addr;

	return memory[addr];
}

void mem_write(uint16_t const addr, uint16_t const data)
{
	if(memory_monitor[addr])
		memory_address_accessed = addr;

	if(addr < 0x8000)
	{
		int x = (addr >> 0) & 0xFF;
		int y = (addr >> 8) & 0xFF;

		if(x > output_win_cols - 3)
			return;
		if(y > output_win_rows - 1)
			return;

		//offset due to title 
		mvwprintw(output_window, y + 1, x + 1, "%c", static_cast<char>(data & 0xFF));
		wrefresh(output_window);
	}
	else
	{
		memory[addr] = data;
	}

	return;
}

int process_instruction(int const keyboard_char)
{
	uint16_t instruction = mem_read(ip);
	ip++;

	uint16_t const opcode = (instruction >> 12) & 0xF;
	uint16_t const funct  = (instruction >>  8) & 0x7;
	
	uint16_t const rd     = (instruction >>  4) & 0xF;
	uint16_t const rs     = (instruction >>  0) & 0xF;

	uint16_t const imm = mem_read(ip & 0xFFFF); //may not actually be needed
	bool const is_imm = (instruction >> 11) & 0x1;

	uint16_t const src = (is_imm ? imm : registers[rs]);

	if(is_imm)
		ip += 1;

	uint16_t reg0_saved = registers[0]; //instead of preventing write, restore at the end, may be modified to clear interrupt


	switch(opcode)
	{
	case 0b0000:
		Log::error("Invalid opcode 0x00");
		return 1;

	case 0b0001:
		registers[rd] = src;
		goto next_instruction;

	case 0b0010:
	{
		uint32_t res;

		switch(funct)
		{
		case 0b000: res = static_cast<uint32_t>(registers[rd]) +  static_cast<uint32_t>(src      ); break;
		case 0b001: res = static_cast<uint32_t>(registers[rd]) -  static_cast<uint32_t>(src      ); break;
		case 0b010: res = static_cast<uint32_t>(registers[rd]) &  static_cast<uint32_t>(src      ); break;
		case 0b011: res = static_cast<uint32_t>(registers[rd]) |  static_cast<uint32_t>(src      ); break;
		case 0b100: res = static_cast<uint32_t>(registers[rd]) ^  static_cast<uint32_t>(src      ); break;
		case 0b101: res =                                      ~  static_cast<uint32_t>(src      ); break;
		//& 0xF ensures shifts shift by at most 16
		case 0b110: res = static_cast<uint32_t>(registers[rd]) << static_cast<uint32_t>(src & 0xF); break;
		case 0b111: res = static_cast<uint32_t>(registers[rd]) >> static_cast<uint32_t>(src & 0xF); break;
		}

		flag_sign      = (res >> 15) & 0b1;
		flag_overflow  = (((registers[rd] ^ src) & ~(src ^ res)) >> 15) & 0b1;
		flag_zero      = (res & 0xFFFF) == 0x0000;
		flag_carry     = (res >> 16) & 0b1;

		registers[rd] = res;
	
		goto next_instruction;
	}
	case 0b0011:
	{
		uint32_t res;

		switch(funct)
		{
		case 0b000:
			Log::error("Invalid funct for opcode=0b0011"); return 1;

		case 0b001: res = static_cast<uint32_t>(registers[rd]) - static_cast<uint32_t>(src); break;
		case 0b010: res = static_cast<uint32_t>(registers[rd]) & static_cast<uint32_t>(src); break;

		case 0b011: case 0b100: case 0b101: case 0b110: case 0b111:
			Log::error("Invalid funct for opcode=0b0011"); return 1;
		}

		flag_sign      = (res >> 15) & 0b1;
		flag_overflow  = (((registers[rd] ^ src) & ~(src ^ res)) >> 15) & 0b1;
		flag_zero      = (res & 0xFFFF) == 0x0000;
		flag_carry     = (res >> 16) & 0b1;
	
		goto next_instruction;
	}

	case 0b0100:
	case 0b0101:
	{
		//it is possible to merge these bits as they are nicely consecutive
		uint16_t const jump_cond = ((opcode & 0b1) << 3) | funct;

		bool should_jump = false;

		switch(jump_cond)
		{
		case 0b0000: should_jump = true; break;
		case 0b0001: should_jump = flag_zero; break;
		case 0b0010: should_jump = not flag_zero; break;
		case 0b0011: should_jump = flag_sign == flag_overflow; break;
		case 0b0100: should_jump = (flag_zero) or (flag_sign != flag_overflow); break;
		case 0b0101: should_jump = (not flag_zero) and (flag_sign == flag_overflow); break;
		case 0b0110: should_jump = flag_sign != flag_overflow; break;
		case 0b0111: should_jump = flag_overflow; break;
		case 0b1000: should_jump = (registers[0] & 0xFF) == 0x00; break; //busy flags
		case 0b1001: should_jump = flag_sign; break;
		case 0b1010: should_jump = not flag_sign; break;
		case 0b1011: should_jump = not flag_carry; break;
		case 0b1100: should_jump = flag_carry or flag_zero; break;
		case 0b1101: should_jump = not flag_carry or not flag_zero; break;
		case 0b1110: should_jump = flag_carry; break;
		case 0b1111: should_jump = not flag_overflow; break;
		}

		if(should_jump)
			ip = src;
			
		goto next_instruction;
	}

	case 0b0110:
		switch(funct)
		{
		case 0b000: reg0_saved &= 0x7FFF; break;
		case 0b001: registers[rd] = keyboard_char; reg0_saved &= 0xBFFF; break;
		case 0b010: reg0_saved &= 0xDFFF; break;
		case 0b011: reg0_saved &= 0xEFFF; break;
		case 0b100: reg0_saved &= 0xF7FF; break;
		case 0b101: reg0_saved &= 0xFBFF; break;
		case 0b110: reg0_saved &= 0xFDFF; break;
		case 0b111: reg0_saved &= 0xFEFF; break;
		}
		goto next_instruction;

	case 0b0111:
		//busy flag is never set, there is no reason to check it
		Log::error("out is currently not needed");

		goto next_instruction;

	case 0b1000:
		registers[rd] = mem_read(src);

		goto next_instruction;

	case 0b1001:
		mem_write(src, registers[rd]);

		goto next_instruction;

	case 0b1010:
		mem_write(registers[2] - 1, ip);

		registers[2] = registers[2] - 1;

		ip = src;

		goto next_instruction;

	case 0b1011:
	{
		switch(funct)
		{
		case 0b000: ip = memory[registers[2]]; registers[2]++; break;
		case 0b001: ip = memory[registers[2]]; registers[2]++; 
			registers[1]  = (int_mask_saved << 8) | (registers[1] & 0xFF);  
			flag_sign     = flags_saved & 0b1000; 
			flag_overflow = flags_saved & 0b0100;
			flag_carry    = flags_saved & 0b0010;
			flag_zero     = flags_saved & 0b0001;
			break;

		case 0b010: case 0b011: case 0b100: case 0b101: case 0b110: case 0b111:
			Log::error("Invalid funct for opcode=0b1011"); return 1;
		}

		goto next_instruction;
	}
	case 0b1100:
	{
		mem_write(registers[2] - 1, src);

		registers[2] = registers[2] - 1;

		goto next_instruction;
	}
	case 0b1101:
	{
		registers[rd] = memory[registers[2]];
		registers[2] = registers[2] + 1;

		goto next_instruction;
	}
	case 0b1110:
			Log::error("Invalid opcode=0b1110"); return 1;
	case 0b1111:
			Log::error("Invalid opcode=0b1111"); return 1;
	}


next_instruction:
	registers[0] = reg0_saved;

	uint16_t const int_occured = (registers[0] & registers[1]) >> 8;

	if(0x0000 == int_occured)
		return 0;

	//save the mask
	int_mask_saved = registers[1] >> 8;
	flags_saved = (static_cast<uint16_t>(flag_sign    ) << 3)
	            | (static_cast<uint16_t>(flag_overflow) << 2)
	            | (static_cast<uint16_t>(flag_carry   ) << 1)
	            | (static_cast<uint16_t>(flag_zero    ) << 0);

	//turn off the mask
	registers[1] = registers[1] & 0xFF;

	//save the ip
	mem_write((registers[2] - 1) & 0xFFFF, ip);
	registers[2]--;

	//switch the ip
	ip = 0xFFF0 + 2 * std::countl_zero(static_cast<uint8_t>(int_occured));

return 0;
}
