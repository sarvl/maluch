#include "../Assembler/asm.h"
#include "../Assembler/asm_helper.h"

#include <cstring>

#include <string_view>


using namespace std::literals;
using namespace reg;

constexpr std::string_view str_shell          = "mALUsh v0.1";
constexpr std::string_view str_true           = "true";
constexpr std::string_view str_false          = "false";
constexpr std::string_view str_cmd_help       = "help";
constexpr std::string_view str_cmd_time       = "time";
constexpr std::string_view str_cmd_help_reply = "help - prints this help\necho string - prints string\ntime - prints elapsed time\nclear - clears the screen";
constexpr std::string_view str_cmd_echo       = "echo";
constexpr std::string_view str_cmd_clear      = "clear";
constexpr std::string_view str_cmd_invalid    = "invalid command";

constexpr uint16_t addr_buffer             = 0xFF00;
constexpr uint16_t addr_buffer_pos         = 0xFEFF;
constexpr uint16_t addr_cmd_ready          = 0xFEFE;
constexpr uint16_t addr_screen_cursor      = 0xFEEF;
constexpr uint16_t addr_print_convert_hex  = 0xFED0;
constexpr uint16_t addr_time_elapsed       = 0xFECF;

constexpr uint16_t addr_str                = 0xF000;
constexpr uint16_t addr_str_true           = addr_str + 0;
constexpr uint16_t addr_str_false          = addr_str_true           + str_true.length();
constexpr uint16_t addr_str_shell          = addr_str_false          + str_false.length();
constexpr uint16_t addr_str_cmd_time       = addr_str_shell          + str_shell.length();
constexpr uint16_t addr_str_cmd_help       = addr_str_cmd_time       + str_cmd_time.length();
constexpr uint16_t addr_str_cmd_help_reply = addr_str_cmd_help       + str_cmd_help.length();
constexpr uint16_t addr_str_cmd_echo       = addr_str_cmd_help_reply + str_cmd_help_reply.length();
constexpr uint16_t addr_str_cmd_clear      = addr_str_cmd_echo       + str_cmd_echo.length();
constexpr uint16_t addr_str_cmd_invalid    = addr_str_cmd_clear      + str_cmd_clear.length();

constexpr int int_timer    = 0;
constexpr int int_keyboard = 1;


void populate_constants()
{
	char const arr_b2h[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	mem_allocate(addr_print_convert_hex, arr_b2h);
	mem_allocate(addr_buffer_pos, addr_buffer);

	mem_allocate(addr_str_true          , str_true          );
	mem_allocate(addr_str_false         , str_false         );
	mem_allocate(addr_str_shell         , str_shell         );
	mem_allocate(addr_str_cmd_time      , str_cmd_time      );
	mem_allocate(addr_str_cmd_help      , str_cmd_help      );
	mem_allocate(addr_str_cmd_help_reply, str_cmd_help_reply);
	mem_allocate(addr_str_cmd_echo      , str_cmd_echo      );
	mem_allocate(addr_str_cmd_clear     , str_cmd_clear     );
	mem_allocate(addr_str_cmd_invalid   , str_cmd_invalid   );
	
	return;
}

void print_str(uint16_t const addr, uint16_t const size)
{
	i_mov(R8, addr);
	i_mov(R9, size);
	i_call("kernel_screen_print_str");
	
	return;
}

void print_str_newline(uint16_t const addr, uint16_t const size)
{
	i_mov(R8, addr);
	i_mov(R9, size);
	i_call("kernel_screen_print_str_nl");
	
	return;
}

void compare_strings(uint16_t const addr0, uint16_t const addr1, uint16_t const length)
{
	i_mov(R8, addr0);
	i_mov(R9, addr1);
	i_mov(R10, length);
	i_call("string_compare");

	return;
}
int code()
{
	populate_constants();

	auto const stack_pointer = R2;

	auto const saved0 = R4;
	auto const saved1 = R5;
	auto const saved2 = R6;
	auto const saved3 = R7;

	auto const temp0 =  R8;
	auto const temp1 =  R9;
	auto const temp2 = R10;
	auto const temp3 = R11;
	auto const temp4 = R12;
	auto const temp5 = R13;
	auto const temp6 = R14;
	auto const temp7 = R15;

	auto const retval = R8;

	i_jmp("start");

	//each one has dumb servicing for now
	label("IHR");

		label("IHR_0");
			i_push(temp0);
			i_in(int_timer, temp0);

			i_ldw(temp0, addr_time_elapsed);
			i_add(temp0, 1);
			i_stw(temp0, addr_time_elapsed);

			i_pull(temp0);
			i_iret();

		label("IHR_1");
		{
			auto const character  = saved0;
			auto const buffer_pos = saved1;

			i_push(saved0);
			i_push(saved1);
			i_push(temp0);

			i_in(int_keyboard, character);

			i_ldw(buffer_pos, addr_buffer_pos);

			i_mov(temp0, character);
			i_call("kernel_screen_print_char");

			i_cmp(character, '\n');
			i_bee("IHR_1_new_cmd");

			i_cmp(character, '\b');
			i_bee("IHR_1_buffer_reduce");

			i_stw(character, buffer_pos);

			i_add(buffer_pos, 1);
			i_cmp(buffer_pos, 0xFFF0);
			i_bll("IHR_1_skip_0");
			i_mov(buffer_pos, 0xFF00);
			label("IHR_1_skip_0");
			i_stw(buffer_pos, addr_buffer_pos);

			i_pull(temp0);
			i_pull(saved1);
			i_pull(saved0);
			i_iret();

			label("IHR_1_new_cmd");

			i_mov(buffer_pos, addr_buffer);
			i_stw(buffer_pos, addr_buffer_pos);

			i_mov(temp0, 1);
			label("check");
			i_stw(temp0, addr_cmd_ready);

			i_pull(temp0);
			i_pull(saved1);
			i_pull(saved0);
			i_iret();

			label("IHR_1_buffer_reduce");

			i_cmp(buffer_pos, 0xFF00);
			i_bee("IHR_1_ret");

			i_sub(buffer_pos, 1);
			i_mov(character, ' ');
			i_stw(character, buffer_pos);

			i_stw(buffer_pos, addr_buffer_pos);

			label("IHR_1_ret");
			i_pull(temp0);
			i_pull(saved1);
			i_pull(saved0);
			i_iret();
		}
		
		label("IHR_INVALID");
		word(0x0000); //invalid instr error

	label("kernel");

		label("kernel_screen");
			
			// temp0 - char to print
			// returns: nothing
			label("kernel_screen_print_char");
			{
				auto const character = temp0;
				auto const cursor    = temp1;

				i_ldw(cursor, addr_screen_cursor);

				i_cmp(character, '\n');
				i_bee("kernel_screen_print_char_newline");
				
				i_cmp(character, '\b');
				i_bee("kernel_screen_print_char_backspace");
				

				i_stw(character, cursor);

				i_add(cursor, 1);

				//ensure cursor is within range
				i_and(cursor, 0x7FFF);
				i_stw(cursor, addr_screen_cursor);

				i_ret();

				label("kernel_screen_print_char_newline");

				i_add(cursor, 0x0100);
				i_and(cursor, 0x7F00);
				i_stw(cursor, addr_screen_cursor);

				i_ret();

				label("kernel_screen_print_char_backspace");

				i_sub(cursor, 1);
				i_and(cursor, 0x7FFF);
				i_stw(cursor, addr_screen_cursor);

				i_mov(character, ' ');
				i_stw(character, cursor);

				i_ret();
			}

			// temp0 - pointer to string
			// temp1 - count
			// returns: nothing
			label("kernel_screen_print_str");
			{
				auto const pointer = temp0;
				auto const count   = temp1;
				
				i_test(count, count);
				i_bee("kernel_screen_print_str_ret");
				
				i_push(pointer);
				i_push(count);

				i_ldw(temp0, pointer);
				i_call("kernel_screen_print_char");

				i_pull(count);
				i_pull(pointer);

				i_add(pointer, 1);
				i_sub(count, 1);

				i_jmp("kernel_screen_print_str");

				label("kernel_screen_print_str_ret");
				i_ret();
			}
			
			// temp0 - pointer to string
			// temp1 - count
			// returns: nothing
			label("kernel_screen_print_str_nl");
			{
				i_call("kernel_screen_print_str");

				i_mov(temp0, '\n');
				i_jmp("kernel_screen_print_char");
			}

			// temp0 - reg to print 
			// returns: nothing
			
			label("kernel_screen_print_reg");
			{
				i_push(saved0);

				auto const reg = temp0;
				auto const reg_copy = saved0;
				
				i_mov(reg_copy, reg);

				i_mov(temp0, 'x');
				i_call("kernel_screen_print_char");

				for(int i = 12; i >= 0; i -= 4)
				{
					i_mov(reg, reg_copy);
					i_lsr(reg, i);
					i_and(reg, 0b1111);
					i_add(reg, addr_print_convert_hex);
					i_ldw(temp0, reg);
					i_call("kernel_screen_print_char");
				}
				
				i_pull(saved0);
				i_ret();
			}

			//bool 
			//returns: nothing
			label("kernel_screen_print_bool");
			{
				i_test(temp0, temp0);
				i_bee("kernel_screen_print_bool_false");

				print_str(addr_str_true, str_true.length());
				i_ret();

				label("kernel_screen_print_bool_false");

				print_str(addr_str_false, str_false.length());
				i_ret();
			}

			//nothing
			//returns: nothing
			label("kernel_screen_clear");
			{
				auto const addr = temp0;
				auto const data = temp1;

				i_mov(addr, 0x7FFF);
				i_mov(data, ' ');
				

				label("kernel_screen_clear_loop");
				i_stw(data, addr);
				i_sub(addr, 1);
				i_bne("kernel_screen_clear_loop");

				i_stw(data, addr);

				//addr is now 0, double use it to reset cursor
				i_stw(addr, addr_screen_cursor);

				i_ret();
			}

	label("string");

		//temp0 - str 0
		//temp1 - str 1
		//temp2 - length
		//returns:  1 if equal, 0 otherwise
		label("string_compare");
		{
			auto const str0_pointer = temp0;
			auto const str1_pointer = temp1;
			auto const length       = temp2;
			
			i_test(length, length);
			i_bee("string_compare_true");
			
			i_ldw(temp3, str0_pointer);
			i_ldw(temp4, str1_pointer);
			
			i_cmp(temp3, temp4);
			i_bne("string_compare_false");

			i_add(str0_pointer, 1);
			i_add(str1_pointer, 1);
			i_sub(length, 1);
			i_jmp("string_compare");

			label("string_compare_true");

			i_mov(temp0, 1);
			i_ret();

			label("string_compare_false");

			i_mov(temp0, 0);
			i_ret();

		}
			


	label("start");

	i_mov(stack_pointer, 0xF000);

	set_iht(int_timer,    "IHR_0");
	set_iht(int_keyboard, "IHR_1");

	for(int i = 2; i < 8; i++)
		set_iht(i, "IHR_INVALID");

	label("shell_start");

	print_str_newline(addr_str_shell, str_shell.length());

	interrupt_enable(int_timer);
	interrupt_enable(int_keyboard);

	label("shell_loop");

		i_mov(temp0, '>');
		i_call("kernel_screen_print_char");

		//clear the buffer
		i_mov(temp0, ' ');
		for(int i = 0; i < 0x100 - 16; i++)
			i_stw(temp0, addr_buffer + i);

		label("shell_cmd_wait");
		i_ldw(saved0, addr_cmd_ready);
		i_test(saved0, saved0);
		i_bee("shell_cmd_wait");

		i_mov(saved0, 0);
		i_stw(saved0, addr_cmd_ready);
		
		//buffer may get overwritten if someone types fast enough
		//oh well, what a shame
		compare_strings(addr_buffer, addr_str_cmd_help, str_cmd_help.size());
		i_test(temp0, temp0);
		i_bne("shell_cmd_help");

		compare_strings(addr_buffer, addr_str_cmd_echo, str_cmd_echo.size());
		i_test(temp0, temp0);
		i_bne("shell_cmd_echo");

		compare_strings(addr_buffer, addr_str_cmd_time, str_cmd_time.size());
		i_test(temp0, temp0);
		i_bne("shell_cmd_time");

		compare_strings(addr_buffer, addr_str_cmd_clear, str_cmd_clear.size());
		i_test(temp0, temp0);
		i_bne("shell_cmd_clear");
		
		print_str_newline(addr_str_cmd_invalid, str_cmd_invalid.size());
		i_jmp("shell_loop");

		label("shell_cmd_help");

		print_str_newline(addr_str_cmd_help_reply, str_cmd_help_reply.size());
		i_jmp("shell_loop");
		
		label("shell_cmd_echo");

		i_mov(temp0, addr_buffer + 5);
		i_mov(temp1, 0xFF - 5);
		i_call("kernel_screen_print_str_nl");

		i_jmp("shell_loop");

		label("shell_cmd_time");
	
		i_ldw(temp0, addr_time_elapsed);
		i_call("kernel_screen_print_reg");
		i_mov(temp0, '\n');
		i_call("kernel_screen_print_char");

		i_jmp("shell_loop");

		label("shell_cmd_clear");
	
		i_call("kernel_screen_clear");

		i_jmp("shell_loop");

	return 0;
}

