#pragma once

#include <ncurses.h>

constexpr short ctermfg_red     = 1;
constexpr short ctermfg_green   = 2;
constexpr short ctermfg_yellow  = 3;
constexpr short ctermfg_blue    = 4;
constexpr short ctermfg_magenta = 5;
constexpr short ctermfg_cyan    = 6;
constexpr short ctermfg_white   = 7;

extern bool resize;

void init_tui();
void window_update(WINDOW* win, char const * const title);

//wrapper to remove unnecessary argument
void wcolor_set(WINDOW* win, short pair);
