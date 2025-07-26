#include <ncurses.h>

#include <signal.h> //capturing resize

//necessary to properly capture screen resize
#include <sys/ioctl.h> 

#include <vector>

#include <string> 
#include <cstring> //strncmp

using namespace std::literals;

bool resize = false;

constexpr short ctermfg_red     = 1;
constexpr short ctermfg_green   = 2;
constexpr short ctermfg_yellow  = 3;
constexpr short ctermfg_blue    = 4;
constexpr short ctermfg_magenta = 5;
constexpr short ctermfg_cyan    = 6;
constexpr short ctermfg_white   = 7;

static void screen_resize(int)
{
	resize = true;
	return;
}

void window_update(WINDOW* win, char const * const title)
{
	box(win, 0, 0);
	mvwprintw(win,   0, 1, title);
	return;
}

int main()
{
	signal(SIGWINCH, screen_resize);
//	setlocale(LC_ALL, "");

	initscr();
	start_color();
	cbreak();
	noecho();
	curs_set(0);

	init_pair(ctermfg_red    , COLOR_RED    , COLOR_BLACK);
	init_pair(ctermfg_green  , COLOR_GREEN  , COLOR_BLACK);
	init_pair(ctermfg_yellow , COLOR_YELLOW , COLOR_BLACK);
	init_pair(ctermfg_blue   , COLOR_BLUE   , COLOR_BLACK);
	init_pair(ctermfg_magenta, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(ctermfg_cyan   , COLOR_CYAN   , COLOR_BLACK);
	init_pair(ctermfg_white  , COLOR_WHITE  , COLOR_BLACK);

	intrflush(stdscr, false);
	keypad(stdscr, true);
	nodelay(stdscr, true);
	
	int cols; int rows;
	getmaxyx(stdscr, rows, cols);

	WINDOW* win_screen    = newwin(rows - 8, cols - 25, 0, 0);
	WINDOW* win_registers = newwin(10, 25, 0, cols - 25);
	WINDOW* win_memory    = newwin(rows - 10, 25, 10, cols - 25);
	WINDOW* win_cmdline   = newwin(8, cols - 25, rows - 8, 0);

	bool is_cmd = true;
	wcolor_set(win_cmdline, ctermfg_cyan, nullptr);
	enum {change_no, change_refresh, change_repaint} change = change_repaint;

	std::string cmd;

	std::vector<std::string> cmd_window_buffer = {"", "", "", "", ""}; //a few strings to prevent out of bounds access
	int cmd_window_print_start = 0;

	while(true)
	{
		//process input
		//	if cmd and enter then evaluate command
		//	if tab then switch modes
		//process instruction
		//	if output necessary, print it 
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

			wresize(win_screen   , rows - 8, cols - 25);
//			wresize(win_registers, 10, 25); not needed
			wresize(win_memory   , rows - 10, 25);
			wresize(win_cmdline  , 8, cols - 25);

//			mvwin(win_screen     , 0, 0); not needed
			mvwin(win_registers  , 0, cols - 25);
			mvwin(win_memory     , 10, cols - 25);
			mvwin(win_cmdline    , rows - 8, 0);

			resize = false;
			change = change_repaint;
		}
			
		if(change_repaint == change)
		{
			clear();
			refresh();

			window_update(win_screen   , "SCREEN"   );
			window_update(win_registers, "REGISTERS");
			window_update(win_memory   , "MEMORY"   );
			window_update(win_cmdline  , "COMMANDS" );

			mvwprintw(win_cmdline, 6, 1, ">%s", cmd.c_str());
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

		auto const input = getch();
		switch(input)
		{
		case '\t':
			is_cmd = not is_cmd;
			//switch which window is highlighted
			
			if(is_cmd)
			{
				wcolor_set(win_cmdline, ctermfg_cyan, nullptr);
				wcolor_set(win_screen , ctermfg_white, nullptr);
				curs_set(2);
			}
			else
			{
				wcolor_set(win_cmdline, ctermfg_white, nullptr);
				wcolor_set(win_screen , ctermfg_cyan, nullptr);
				curs_set(0);
			}

			change = change_repaint;
			break;

		case 'A' ... 'Z':
		case 'a' ... 'z':
		case '0' ... '9':
		case ' ':
			if(is_cmd)
			{
				cmd += input;

				mvwprintw(win_cmdline, 6, 1, ">%s", cmd.c_str());
				wrefresh(win_cmdline);
			}
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

				break;
			}
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

				break;
			}

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
					cmd_window_buffer.emplace_back("help - displays this help");
					cmd_window_buffer.emplace_back("exit - exits the simulator");
					cmd_window_buffer.emplace_back("clear - clears current buffer");
					cmd_window_buffer.emplace_back("line4");
					cmd_window_buffer.emplace_back("line5");
					cmd_window_buffer.emplace_back("line6");

				}
				else if(0 == strncmp(cmd.c_str(), "clear ", 6))
				{
					cmd_window_buffer = {"", "", "", "", ""};
				}
				else 
				{
					cmd_window_buffer.emplace_back("invalid command");
				}

				//always reset after new command
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

				break;
			}
		}
		
//		if(process_instruction(win_screen, memory[ip], memory[ip + 1]))
//		{
//			error = true;
//			goto finish;
//		}
	}

finish:
	(void)delwin(win_screen   );
	(void)delwin(win_registers);
	(void)delwin(win_memory   );
	(void)delwin(win_cmdline  );

	endwin();

	return 0;
//	if(not error)
//		return 0;
//
//	printf("ERROR: invalid instruction @ 0x%04X\n", ip);
}
