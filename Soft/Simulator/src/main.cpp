#include <ncurses.h>

#include <signal.h> //capturing resize

//necessary to properly capture screen resize
#include <sys/ioctl.h> 

#include <vector>

#include <string> 
#include <cstring> //strncmp

#include <chrono>

#include <fcntl.h> //open, close
#include <unistd.h> //write

#include <unordered_map> 

#include <format>

#include "tui.h"

using namespace std::literals;
using namespace std::chrono;

extern uint16_t memory[1 << 16];
extern uint16_t memory_monitor[1 << 16];
extern uint16_t registers[16];
extern uint16_t iht[8];
extern uint16_t ip;
extern uint16_t int_mask_saved;
extern bool flag_sign;
extern bool flag_overflow;
extern bool flag_carry;
extern bool flag_zero;
extern WINDOW* output_window;

extern int memory_address_accessed;

int breakpoints[1 << 16];

int output_win_cols, output_win_rows;

int process_instruction(int const keyboard_char);
int read_file(uint16_t * const dest, std::string_view const name);

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

int read_hexd(char const* const start)
{
	int const parts[4] = {
		hexd_to_val(*(start + 0)),
		hexd_to_val(*(start + 1)),
		hexd_to_val(*(start + 2)),
		hexd_to_val(*(start + 3))};
	
	if(-1 == parts[0]
	|| -1 == parts[1]
	|| -1 == parts[2]
	|| -1 == parts[3])
		return -1;

	return (parts[0] << 12)
	     | (parts[1] <<  8)
	     | (parts[2] <<  4)
	     | (parts[3] <<  0)
	     ;
}

int main(int const argc, char const * const argv[])
{
	if(read_file(memory, (1 == argc ? "out.bin" : argv[1])))
		return 1;

	init_tui();	

	int cols; int rows;
	getmaxyx(stdscr, rows, cols);

	WINDOW* win_screen    = newwin(rows - 8, cols - 27, 0, 0);
	WINDOW* win_registers = newwin(10, 27, 0, cols - 27);
	WINDOW* win_memory    = newwin(rows - 10, 27, 10, cols - 27);
	WINDOW* win_cmdline   = newwin(8, cols - 27, rows - 8, 0);

	bool is_cmd = true;
	wcolor_set(win_cmdline, ctermfg_cyan);
	wcolor_set(win_registers, ctermfg_yellow);
	wcolor_set(win_memory, ctermfg_yellow);
	enum {change_no, change_refresh, change_repaint} change = change_repaint;

	std::string cmd;

	std::vector<std::string> cmd_window_buffer = {"", "", "", "", ""}; //a few strings to prevent out of bounds access
	unsigned int cmd_window_print_start = 0;

	bool error = false;

	auto timestamp = std::chrono::high_resolution_clock::now();	
	int keyboard_char = 0; //getch returns int

	output_window = win_screen;
	output_win_cols = cols - 27;
	output_win_rows = rows - 10;

	enum {base_ip, base_custom} mode_mem_print    = base_ip;
	uint16_t mem_start_saved = 0x0000;

	bool paused = true;
	bool was_paused = true;
	bool cmd_screen_update = true;
	int exec_count = 0;

	breakpoints[0x0000] = -1;

	while(true)
	{
		//if not in breakpoint
		//	process instruction
		//	if output
		//		print
		//	
		//process input (and set keyboard interrupt)
		//	if cmd and enter then evaluate command
		//	if tab then switch modes
		//check the rest of interrupts
		//print registers
		//print memory 
		
		//this most likely is not thread safe
		//but it also probably does not matter
		//	1 rarely missed event and spurious resize (to the same size) are not a problem
		//will consider fixing when a problem is demonstrated
		
		if(resize)
		{
			winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
			cols = w.ws_col;
			rows = w.ws_row;
			resize_term(rows, cols);

			clear();
			//resized stuff has to be cleared too
			wclear(win_screen);
			wclear(win_memory);
			wclear(win_cmdline);
			refresh();

			wresize(win_screen   , rows - 8, cols - 27);
//			wresize(win_registers, 10, 25); not needed
			wresize(win_memory   , rows - 10, 27);
			wresize(win_cmdline  , 8, cols - 27);

//			mvwin(win_screen     , 0, 0); not needed
			mvwin(win_registers  , 0, cols - 27);
			mvwin(win_memory     , 10, cols - 27);
			mvwin(win_cmdline    , rows - 8, 0);


			output_win_cols = cols - 27;
			output_win_rows = rows - 10;
			resize = false;
			change = change_repaint;
		}
			
		if(change_repaint == change)
		{
			clear();
			refresh();

			wcolor_set(win_registers, ctermfg_white);
			wcolor_set(win_memory, ctermfg_white);

			window_update(win_screen   , "SCREEN"   );
			window_update(win_registers, "REGISTERS");
			window_update(win_memory   , "MEMORY"   );
			window_update(win_cmdline  , "COMMANDS" );

			mvwprintw(win_cmdline, 6, 1, ">%s", cmd.c_str());

			wcolor_set(win_registers, ctermfg_yellow);
			wcolor_set(win_memory, ctermfg_yellow);
		}
		if(change_repaint == change
		|| change_refresh == change)
		{
			//wmove does not work for some reason
			if(is_cmd)
				move(rows - 2, cmd.size() + 2);

			wrefresh(win_screen    );
			wrefresh(win_registers );
			wrefresh(win_memory    );
			wrefresh(win_cmdline   );

			refresh();
			change = change_no;

		}

		uint16_t const prev_ip = ip;

		if(not paused)
		{
			if(process_instruction(keyboard_char))
			{
				error = true;
				goto finish;
			}
		}
		else if(exec_count > 0)
		{
			if(process_instruction(keyboard_char))
			{
				error = true;
				goto finish;
			}

			exec_count--;
		}

		uint16_t const next_ip = ip;

		was_paused = paused;

		if(not paused && breakpoints[next_ip] < 0)
		{
			cmd_window_buffer.emplace_back(std::format("Break at {:x}", next_ip));
			cmd_screen_update = true;

			paused = true;
			//no need to continuously update stuff 
			nodelay(stdscr, false);
		}
		
		if(-1 != memory_address_accessed)
		{
			cmd_window_buffer.emplace_back(std::format("Accessed {:x} by IP={:x}", memory_address_accessed, prev_ip));	
			cmd_screen_update = true;

			memory_address_accessed = -1;
		}	

		//interrupts
		auto const cur_time = std::chrono::high_resolution_clock::now();
		auto const time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - timestamp).count();
		if(time_diff >= 1000)
		{
			registers[0] |= (0b1 << (8 + 7));
			timestamp = cur_time;
		}

		if(paused || was_paused || 0 == (time_diff & 0x3F))
		{
			//display registers
			//first is special
			mvwprintw(win_registers, 1, 1, "R00 %04X R08 %04X IP %04X", registers[0], registers[0+8], ip);
			for(int i = 1; i < 8; i++)
				mvwprintw(win_registers, 1 + i, 1, "R%02d %04X R%02d %04X", i, registers[i], i + 8, registers[i+8]);

			wrefresh(win_registers);

			//display memory
			uint16_t mem_start;
			if(mode_mem_print == base_ip)
			{
				mvwprintw(win_memory, 1, 1, "ADDRESS: IP=%04X", ip);
				mem_start = ip;
			}
			else if(mode_mem_print == base_custom)
			{
				mvwprintw(win_memory, 1, 1, "ADDRESS: CUSTOM=%04X", mem_start_saved);
				mem_start = mem_start_saved;
			}


			for(int i = 0; i < rows - 13; i++)
				mvwprintw(win_memory, 2 + i, 1, "%04X %04X    %04X %04X",
					(mem_start + i) & 0xFFFF, memory[(mem_start + i) & 0xFFFF],
					(mem_start + i + rows - 13) & 0xFFFF, memory[(mem_start + i + rows - 13) & 0xFFFF]);

			wrefresh(win_memory);
		}

		if(cmd_screen_update)
		{
			cmd_window_print_start = cmd_window_buffer.size() - 5;

			//clear previous window
			wclear(win_cmdline);
			wrefresh(win_cmdline);
			window_update(win_cmdline, "COMMANDS");

			//update the display
			for(int i = 5; i > 0; i--)
				mvwprintw(win_cmdline, 6 - i, 1, cmd_window_buffer[cmd_window_print_start + 5 - i].c_str());

			move(6, 2);	
			wrefresh(win_cmdline);

			cmd_screen_update = false;
		}

		auto const input = getch();
		switch(input)
		{
		case '\t':
			is_cmd = not is_cmd;
			//switch which window is highlighted
			
			if(is_cmd)
			{
				wcolor_set(win_cmdline, ctermfg_cyan);
				wcolor_set(win_screen , ctermfg_white);
				curs_set(2);
			}
			else
			{
				wcolor_set(win_cmdline, ctermfg_white);
				wcolor_set(win_screen , ctermfg_cyan);
				curs_set(0);
			}

			change = change_repaint;
			break;

		//surpress range warn
		#pragma GCC diagnostic ignored "-Wpedantic"
		case 'A' ... 'Z':
		case 'a' ... 'z':
		case '0' ... '9':
		case ' ':
		#pragma GCC diagnostic pop
			if(is_cmd)
			{
				cmd += input;

				mvwprintw(win_cmdline, 6, 1, ">%s", cmd.c_str());
				wrefresh(win_cmdline);
			}
			else
				goto set_interrupt_keyboard;
			break;
		case 127:
		case KEY_BACKSPACE:
		case '\b':
			if(is_cmd)
			{
				if(cmd.size() > 0) cmd.pop_back();

				mvwprintw(win_cmdline, 6, cmd.size() + 2, " ");
				move(rows - 2, cmd.size() + 2);
				wrefresh(win_cmdline);
			}
			else
				goto set_interrupt_keyboard;

			break;
		case KEY_UP:
			if(is_cmd)
			{
				if(cmd_window_print_start > 0)
					cmd_window_print_start--;

				//clear previous window
				wclear(win_cmdline);
				wrefresh(win_cmdline);
				window_update(win_cmdline, "COMMANDS");

				//update the display
				for(int i = 5; i > 0; i--)
					mvwprintw(win_cmdline, 6 - i, 1, cmd_window_buffer[cmd_window_print_start + 5 - i].c_str());

				move(rows - 2, cmd.size() + 2);
				wrefresh(win_cmdline);
			}
			break;

		case KEY_DOWN:
			if(is_cmd)
			{
				if(cmd_window_print_start < cmd_window_buffer.size() - 5)
					cmd_window_print_start++;

				//clear previous window
				wclear(win_cmdline);
				wrefresh(win_cmdline);
				window_update(win_cmdline, "COMMANDS");

				//update the display
				for(int i = 5; i > 0; i--)
					mvwprintw(win_cmdline, 6 - i, 1, cmd_window_buffer[cmd_window_print_start + 5 - i].c_str());

				move(rows - 2, cmd.size() + 2);
				wrefresh(win_cmdline);

			}
			break;

		case '\n':
			if(is_cmd)
			{
				//move cmd into buffer
				cmd += ' ';
				cmd_window_buffer.emplace_back(std::move(cmd));
				cmd.clear();
				auto const& cmd = cmd_window_buffer.back();


				//process commands
				     if(0 == strncmp(cmd.c_str(), "exit ", 5)
				     || 0 == strncmp(cmd.c_str(), "quit ", 5)
				     || 0 == strncmp(cmd.c_str(), "q ",    2))
					goto finish;
				else if(0 == strncmp(cmd.c_str(), "help ", 5)
				     || 0 == strncmp(cmd.c_str(), "h "   , 2))
				{
					cmd_window_buffer.emplace_back("help           - displays this help");
					cmd_window_buffer.emplace_back("exit           - exits the simulator");
					cmd_window_buffer.emplace_back("clear          - clears current buffer");
					cmd_window_buffer.emplace_back("break XXXX     - sets breakpoint to address XXXX in hexadecimal");
					cmd_window_buffer.emplace_back("nobreak XXXX   - removes breakpoint from address XXXX");
					cmd_window_buffer.emplace_back("monitor XXXX   - monitors address XXXX, when accessed sets breakpoint");
					cmd_window_buffer.emplace_back("nomonitor XXXX - stops monitoring address XXXX");
					cmd_window_buffer.emplace_back("show XXXX      - shows contents of memory at address XXXX");
					cmd_window_buffer.emplace_back("run            - continue from breakpoint");
					cmd_window_buffer.emplace_back("step           - steps to next instruction");
//					cmd_window_buffer.emplace_back("next           - like step but does NOT descend into call");
					cmd_window_buffer.emplace_back("");

				}
				else if(0 == strncmp(cmd.c_str(), "clear ", 6))
				{
					cmd_window_buffer = {"", "", "", "", ""};
				}
				else if(0 == strncmp(cmd.c_str(), "run ", 4))
				{
					paused = false;
					nodelay(stdscr, true);
				}
				else if(0 == strncmp(cmd.c_str(), "break ", 6))
				{
					if(cmd.size() != 11)
					{
						cmd_window_buffer.emplace_back("ERROR: break expects 4 digit hexadecimal number");
						cmd_window_buffer.emplace_back("");
					}
					else
					{
						int const addr = read_hexd(cmd.c_str() + 6);

						if(-1 == addr)
						{
							cmd_window_buffer.emplace_back("ERROR: invalid hexadecimal character");
							cmd_window_buffer.emplace_back("");
						}
						else
						{
							breakpoints[addr] = -1;
						}
					}	
				}
				else if(0 == strncmp(cmd.c_str(), "show ", 5))
				{
					if(cmd.size() != 10)
					{
						cmd_window_buffer.emplace_back("ERROR: show expects 4 digit hexadecimal number");
						cmd_window_buffer.emplace_back("");
					}
					else
					{
						int const addr = read_hexd(cmd.c_str() + 5);

						if(-1 == addr)
						{
							cmd_window_buffer.emplace_back("ERROR: invalid hexadecimal character");
							cmd_window_buffer.emplace_back("");
						}
						else
						{
							char data[32];
							snprintf(data, 32, "M[x%04X] = x%04X = %d", addr, memory[addr], memory[addr]);
							cmd_window_buffer.emplace_back(data);
						}
					}	
				}
				else if(0 == strncmp(cmd.c_str(), "nobreak ", 8)
				     || 0 == strncmp(cmd.c_str(), "unbreak ", 8))
				{
					if(cmd.size() != 13)
					{
						cmd_window_buffer.emplace_back("ERROR: nobreak expects 4 digit hexadecimal number");
						cmd_window_buffer.emplace_back("");
					}
					else
					{
						int const addr = read_hexd(cmd.c_str() + 8);

						if(-1 == addr)
						{
							cmd_window_buffer.emplace_back("ERROR: invalid hexadecimal character");
							cmd_window_buffer.emplace_back("");
						}
						else
						{
							breakpoints[addr] = 0;
						}
					}	
				}
				else if(0 == strncmp(cmd.c_str(), "monitor ", 8))
				{
					if(cmd.size() != 13)
					{
						cmd_window_buffer.emplace_back("ERROR: break expects 4 digit hexadecimal number");
						cmd_window_buffer.emplace_back("");
					}
					else
					{
						int const addr = read_hexd(cmd.c_str() + 8);

						if(-1 == addr)
						{
							cmd_window_buffer.emplace_back("ERROR: invalid hexadecimal character");
							cmd_window_buffer.emplace_back("");
						}
						else
						{
							memory_monitor[addr] = 1;
						}
					}	
				}
				else if(0 == strncmp(cmd.c_str(), "nomonitor ", 10)
				     || 0 == strncmp(cmd.c_str(), "unmonitor ", 10))
				{
					if(cmd.size() != 15)
					{
						cmd_window_buffer.emplace_back("ERROR: nobreak expects 4 digit hexadecimal number");
						cmd_window_buffer.emplace_back("");
					}
					else
					{
						int const addr = read_hexd(cmd.c_str() + 10);

						if(-1 == addr)
						{
							cmd_window_buffer.emplace_back("ERROR: invalid hexadecimal character");
							cmd_window_buffer.emplace_back("");
						}
						else
						{
							memory_monitor[addr] = 0;
						}
					}	
				}
				else if(0 == strncmp(cmd.c_str(), "step ", 5))
				{
					exec_count = 1;
					paused = true;
				}
//				else if(0 == strncmp(cmd.c_str(), "next ", 5))
//				{
//					if(0b1010 == (memory[next_ip] >> 12))
//						breakpoints[next_ip] = 1;
//					else
//						breakpoints[prev_ip + 1] = 1;
//
//					paused = false;
//				}
				else 
				{
					cmd_window_buffer.emplace_back("invalid command");
					cmd_window_buffer.emplace_back("");
				}

				cmd_screen_update = true;

				break;
			}
			else
				goto set_interrupt_keyboard;

		default:
			break;
		set_interrupt_keyboard:
			registers[0] |= (0b1 << (8 + 6));

			if(KEY_BACKSPACE == input
			|| 127           == input)
			{
				keyboard_char = '\b';
			}
			else
				keyboard_char = static_cast<uint16_t>(input);
		}


		continue;
	}

finish:
	(void)delwin(win_screen   );
	(void)delwin(win_registers);
	(void)delwin(win_memory   );
	(void)delwin(win_cmdline  );

	endwin();

	int const memdump_fd = open("dump.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);							
	char to_write[6] = "    \n";
	for(int i = 0; i < (1 << 16); i++)
	{
		snprintf(to_write, 5, "%04X", memory[i]);
		to_write[4] = '\n';
		write(memdump_fd, to_write, 5);
	}

	close(memdump_fd);

	if(not error)
		return 0;

	printf("ERROR: invalid instruction @ 0x%04X\n", ip);
	return 2;
}
