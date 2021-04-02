#ifndef CONSOLE_H
#define CONSOLE_H

#if !(defined _WIN32 || defined __linux)
	#error only for windows or linux
#endif

#include <stdbool.h>

#if defined _WIN32
#include <windows.h>
typedef COORD Coordinate;
#elif defined __linux
typedef struct {
	short X;
	short Y;
} Coordinate;
#endif

#if defined _WIN32
typedef enum {
	BLACK   = 0,
	BLUE    = 9,
	GREEN   = 10,
	CYAN    = 11,
	RED     = 12,
	MAGENTA = 13,
	YELLOW  = 14,
	WHITE   = 15
} COLOR;
#elif defined __linux
typedef enum {
	BLACK   = 0,
	RED     = 1,
	GREEN   = 2,
	YELLOW  = 3,
	BLUE    = 4,
	MAGENTA = 5,
	CYAN    = 6,
	WHITE   = 7
} COLOR;
#endif

typedef enum {
	UP,
	DOWN,
	RIGHT,
	LEFT,
	SELECT,
	AUTO,
	PAUSE,
	UNDEFINED
} KEY;

void console__init();
void console__fini();
void console__print_map(int passed_second, int score, int left_hint);
void console__move(KEY arrow);
KEY  console__get_selection_key();
void console__update_time(int passed_second);
void console__update_score(int score);
void console__update_hint(int left_hint);
void console__print_tile(Coordinate pos);
void console__print_empty_tile();
void console__delete_tile(Coordinate pos);
void console__flash_path(const Coordinate path[4], int num_point);
void console__print_path(const Coordinate path[4], int num_point);
void console__gotomap(Coordinate pos);
void console__gotoxy(Coordinate pos);
void console__set_color_of(Coordinate pos);
void console__set_color(COLOR fg, COLOR bg);
char console__get_key();
bool console__kbhit();
void console__clear();
void console__set_echo(bool echo);
void console__set_cursor_visibility(bool visibility);
void console__sleep(int ms);
Coordinate console__map2console(Coordinate pos);

#endif // CONSOLE_H
