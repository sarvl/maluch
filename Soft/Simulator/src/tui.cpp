#include "tui.h"

#include <signal.h>

bool resize = false;

static void screen_resize(int)
{
	resize = true;
	return;
}

void init_tui()
{
	signal(SIGWINCH, screen_resize);

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

	return;
}

void window_update(WINDOW* win, char const * const title)
{
	box(win, 0, 0);
	mvwprintw(win,   0, 1, title);
	return;
}

void wcolor_set(WINDOW* win, short pair)
{
	wcolor_set(win, pair, nullptr);
	return;
}

