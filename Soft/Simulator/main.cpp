#include <iostream>

#include <fcntl.h> //open, close
#include <unistd.h> //write

#include <string>
#include <string_view>

#include <unordered_set> //unordered_set
#include <unordered_map> //unordered_set
#include <memory> //unique_ptr

#include <cstdint>
#include <cstdio>
#include <cstring> //strncmp, memcpy

#include "./../utility/src/log.h"
#include "./../utility/src/file.h"

using namespace std::literals;

uint16_t memory[1 << 16];
uint16_t registers[16];
uint16_t ip;
bool flag_sign;
bool flag_overflow;
bool flag_carry;
bool flag_zero;

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

int main(int const argc, char const* const argv[])
{
	std::string_view const file_name = (1 == argc ? "out.bin" : argv[1]);

	File::t_File file;
	if(-1 == File::create_error_handled(&file, file_name))
		return -1;
	char const* const data = file.data;
	int  const        size = file.size;

	//bool debug = 0;
	bool print = 0;

	//placeholder for better interface
	for(int i = 2; i < argc; i++)
	{
//		if(0 == strncmp(argv[i], "-d", 2))
//			debug = true;
		if(0 == strncmp(argv[i], "-p", 2))
			print = true;
	}

	int line_num = 1;
	int off = 0;
	while(off < size)
	{
		if(off + 5 > size
		|| data[off + 0] == '\n'
		|| data[off + 1] == '\n'
		|| data[off + 2] == '\n'
		|| data[off + 3] == '\n')
		{
			Log::error(
				"line does not contain full instruction, aborting",
				file_name,
				line_num);
			
			//not handled because main error is above
			//if something fails, will be handled by OS on cleanup
			(void)File::destroy(file);
			return -2;
		}
		if(data[off + 4] != '\n')
		{
			Log::error(
				"line contains too many characters, aborting",
				file_name,
				line_num);

			(void)File::destroy(file);
			return -3;
		}

		int const parts[4] = {
			hexd_to_val(data[off + 0]),
			hexd_to_val(data[off + 1]),
			hexd_to_val(data[off + 2]),
			hexd_to_val(data[off + 3])};

		if(-1 == parts[0]
		|| -1 == parts[1]
		|| -1 == parts[2]
		|| -1 == parts[3])
		{
			Log::error(
				"line contains character that is not hexadecimal digit, aborting",
				file_name,
				line_num);
			
			//not handled because main error is above
			//if something fails, will be handled by OS on cleanup
			(void)File::destroy(file);
			return -4;
		}


		unsigned const instruction = 0
			| (parts[0] << 12)
			| (parts[1] <<  8)
			| (parts[2] <<  4)
			| (parts[3] <<  0)
			;

		memory[line_num - 1] = instruction;

		line_num++;
		off += 5;
	}
	if(-1 == File::destroy_error_handled(file))
		return -5;

	while(true)
	{
		uint16_t instruction = memory[ip];
		ip++;

		uint16_t const opcode = (instruction >> 12) & 0xF;
		uint16_t const funct  = (instruction >>  8) & 0x7;
		
		uint16_t const rd     = (instruction >>  4) & 0xF;
		uint16_t const rs     = (instruction >>  0) & 0xF;

		uint16_t const imm = memory[ip & 0xFFFF]; //may not actually be needed
		bool const is_imm = (instruction >> 11) & 0x1;

		uint16_t const src = (is_imm ? imm : registers[rs]);

		if(is_imm)
			ip += 1;

		uint16_t const reg0_saved = registers[0]; //instead of preventing write, restore at the end
		switch(opcode)
		{
		case 0b0000:
			Log::error("Invalid opcode 0x00");
			goto finish;

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
				Log::error("Invalid funct for opcode=0b0011"); goto finish;

			case 0b001: res = static_cast<uint32_t>(registers[rd]) - static_cast<uint32_t>(src); break;
			case 0b010: res = static_cast<uint32_t>(registers[rd]) & static_cast<uint32_t>(src); break;

			case 0b011: case 0b100: case 0b101: case 0b110: case 0b111:
				Log::error("Invalid funct for opcode=0b0011"); goto finish;
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
			case 0b0011: should_jump = flag_sign == flag_zero; break;
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
			//not implemented yet
			goto next_instruction;

		case 0b0111:
			//not implemented yet
			goto next_instruction;

		case 0b1000:
			//loading from memory works as expected
			registers[rd] = memory[src];

			goto next_instruction;

		case 0b1001:
			//storing works differently 
			//for purposes of simulator entire VRAM prints low byte via stdout
			//upper part works as usual

			if(src < 0x8000)
				std::cout << static_cast<char>(registers[rd] & 0xFF);
			else
				memory[src] = registers[rd]; 

			goto next_instruction;

		case 0b1010:
			//be careful with storing

			if(registers[2] < 0x8000)
				std::cout << static_cast<char>(ip & 0xFF);
			else
				memory[registers[2]] = ip;

			registers[2] = registers[2] - 1;

			ip = src;

			goto next_instruction;

		case 0b1011:
		{
			switch(funct)
			{
			case 0b000: ip = memory[registers[2]]; registers[2] = registers[2] + 1; break;
			case 0b001: //not implemented yet 

			case 0b010: case 0b011: case 0b100: case 0b101: case 0b110: case 0b111:
				Log::error("Invalid funct for opcode=0b1011"); goto finish;
			}

			goto next_instruction;
		}
		case 0b1100:
		{
			//be careful with storing

			if(registers[2] - 1 < 0x8000)
				std::cout << static_cast<char>(src & 0xFF);
			else
				memory[registers[2] - 1] = src;

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
				Log::error("Invalid opcode=0b1110"); goto finish;
		case 0b1111:
				Log::error("Invalid opcode=0b1111"); goto finish;
		}
 

	next_instruction:
		registers[0] = reg0_saved;
		continue;
		
 	}

finish:
	if(print)
	{
		std::cout << "registers:\n";
		for(int i = 0; i < 16; i++)
			std::cout << "R" << i << " " << registers[i] << '\n';
	}
	int const memdump_fd = open("dump.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);							
	char to_write[6] = "    \n";
	for(int i = 0; i < (1 << 16); i++)
	{
		snprintf(to_write, 5, "%04X", memory[i]);
		to_write[4] = '\n';
		write(memdump_fd, to_write, 5);
	}

	close(memdump_fd);

	return 0;
}
