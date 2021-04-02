#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#if defined _WIN32
	#include <windows.h>
	#include <conio.h>
	#include <fcntl.h>
	#include <io.h>
#elif defined __linux
	#include <sys/ioctl.h>
	#include <unistd.h>
	#include <termios.h>
#endif

#include "console.h"
#include "engine.h"
#include "util.h"

typedef struct {
	wchar_t shape;
	COLOR color;
} Suit;

/**
 * unicode of poker suits
 * https://www.unicode.org/charts/PDF/U25A0.pdf
 * https://www.unicode.org/charts/PDF/U2600.pdf
 */
const Suit tile_shape_table[NUM_TILE_SUIT] = { // in console
	{0x2660, WHITE}, {0x2660, RED}, {0x2660, GREEN}, {0x2660, BLUE},
	{0x2661, WHITE}, {0x2661, RED}, {0x2661, GREEN}, {0x2661, BLUE},
	{0x25C6, WHITE}, {0x25C6, RED}, {0x25C6, GREEN}, {0x25C6, BLUE},
	{0x2663, WHITE}, {0x2663, RED}, {0x2663, GREEN}, {0x2663, BLUE},
	{0x2664, WHITE}, {0x2664, RED}, {0x2664, GREEN}, {0x2664, BLUE},
	{0x2665, WHITE}, {0x2665, RED}, {0x2665, GREEN}, {0x2665, BLUE},
	{0x25C7, WHITE}, {0x25C7, RED}, {0x25C7, GREEN}, {0x25C7, BLUE},
	{0x2667, WHITE}, {0x2667, RED}, {0x2667, GREEN}, {0x2667, BLUE}
};

extern int map_row;
extern int map_col;
extern Coordinate cursor;
extern Coordinate selection;
extern bool block_map_boundary;
extern FILE *log_file;

void console__init() {
#if defined _WIN32
	_setmode(_fileno(stdout), _O_U8TEXT);
#elif defined __linux
	setlocale(LC_CTYPE, "");
#endif

	console__set_echo(false);
	console__set_cursor_visibility(false);
	console__clear();
}

void console__fini() {
	console__set_echo(true);
	console__set_cursor_visibility(true);
	console__set_color(WHITE, BLACK);
	console__clear();
}

void console__print_map(int passed_second, int score, int left_hint) {
	Coordinate pos;

	for(pos.X=1; pos.X<=map_row; pos.X++) {
		for(pos.Y=1; pos.Y<=map_col; pos.Y++) {
			console__set_color_of(pos);
			console__print_tile(pos);
		}
	}

	console__update_time(passed_second);
	console__update_score(score);
	console__update_hint(left_hint);

	pos.X = map_row + 3;
	pos.Y = 5;
	console__gotomap(pos);

	console__set_echo(true);
	wprintf(L"move   : arrow key or wasd");
	pos.Y += 1; console__gotomap(pos);
	wprintf(L"select : space or enter key");
	pos.Y += 1; console__gotomap(pos);
	wprintf(L"hint   : o");
	pos.Y += 1; console__gotomap(pos);
	wprintf(L"pause  : p");
	pos.Y += 2; console__gotomap(pos);
	wprintf(L"if console has broken, pause and resume");
	fflush(stdout);
	console__set_echo(false);
}

void console__move(KEY arrow) {
	Coordinate prev = cursor;

	switch (arrow) {
		case UP:    cursor.Y--; if(cursor.Y < 1)       cursor.Y = block_map_boundary ? 1 : map_col; break;
		case DOWN:  cursor.Y++; if(cursor.Y > map_col) cursor.Y = block_map_boundary ? map_col : 1; break;
		case RIGHT: cursor.X++; if(cursor.X > map_row) cursor.X = block_map_boundary ? map_row : 1; break;
		case LEFT:  cursor.X--; if(cursor.X < 1)       cursor.X = block_map_boundary ? 1 : map_row; break;
		default: break;
	}

	console__set_color_of(prev);
	console__print_tile(prev);
	console__set_color_of(cursor);
	console__print_tile(cursor);
}

void console__print_tile(Coordinate pos) {
	console__gotomap(pos);
	Tile tile = engine__get_tile(pos);
	if(tile == -1) {
		console__print_empty_tile();
	} else {
		console__set_echo(true);
		wprintf(L"%lc", tile_shape_table[tile].shape);
		fflush(stdout);
		console__set_echo(false);
	}
}

void console__print_empty_tile() {
	console__set_echo(true);
#if defined _WIN32
	wprintf(L"  ");
#elif defined __linux
	wprintf(L" ");
#endif
	fflush(stdout);
	console__set_echo(false);
}

void console__update_time(int passed_second) {
	Coordinate pos = {map_row+3, 1};
	pos = console__map2console(pos);

	console__gotoxy(pos);
	console__set_color(WHITE, BLACK);
	console__set_echo(true);
	wprintf(L"time : %d", passed_second); fflush(stdout);
	console__set_echo(false);
	fprintf(log_file, "[%s] %d\n", __func__, passed_second); fflush(log_file);
}

void console__update_score(int score) {
	Coordinate pos = {map_row+3, 2};
	pos = console__map2console(pos);

	console__gotoxy(pos);
	console__set_color(WHITE, BLACK);
	console__set_echo(true);
	wprintf(L"score : %d    ", score); fflush(stdout);
	console__set_echo(false);
	fprintf(log_file, "[%s] %d\n", __func__, score); fflush(log_file);
}

void console__update_hint(int left_hint) {
	if(left_hint == -1) return;

	Coordinate pos = {map_row+3, 3};
	pos = console__map2console(pos);

	console__gotoxy(pos);
	console__set_color(WHITE, BLACK);
	console__set_echo(true);
	wprintf(L"hint : %d    ", left_hint); fflush(stdout);
	console__set_echo(false);
	fprintf(log_file, "[%s] %d\n", __func__, left_hint); fflush(log_file);
}

void console__flash_path(const Coordinate path[4], int num_point) {
	assert(engine__is_available_path(path, num_point), "try to flash broken path");

	// print start and end of path
	Tile tile = engine__get_tile(path[0]);
	console__set_color(tile_shape_table[tile].color, MAGENTA);
	console__print_tile(path[0]);
	tile = engine__get_tile(path[num_point-1]);
	console__set_color(tile_shape_table[tile].color, MAGENTA);
	console__print_tile(path[num_point-1]);

	// print path and overwrite to delete
	console__set_color(WHITE, YELLOW);
	console__print_path(path, num_point);
	console__sleep(100);
	console__set_color(WHITE, BLACK);
	console__print_path(path, num_point);

	// cursor background color can be removed
	console__set_color(WHITE, CYAN);
	console__print_tile(cursor);
}

void console__print_path(const Coordinate path[4], int num_point) {
	int i;
	Coordinate check, start, end;

	for(i=1; i<num_point; i++) {
		check = start = path[i-1];
		end = path[i];

		if(start.X == end.X) {
			while(check.Y != end.Y) {
				fprintf(log_file, "[%s] in %dth vertical segment - (%d,%d)\n", __func__, i, check.X, check.Y); fflush(log_file);
				// pass if is first coordinate
				if( ! (i == 1 && check.Y == start.Y)) {
					console__gotomap(check);
					console__print_empty_tile();
				}

				check.Y = (start.Y < end.Y) ? check.Y + 1 : check.Y - 1;
			}
		}
		else if(start.Y == end.Y) {
			while(check.X != end.X) {
				fprintf(log_file, "[%s] in %dth horizontal segment - (%d,%d)\n", __func__, i, check.X, check.Y); fflush(log_file);
				// pass if is first coordinate
				if( ! (i == 1 && check.X == start.X)) {
					console__gotomap(check);
					console__print_empty_tile();
				}

				check.X = (start.X < end.X) ? check.X + 1 : check.X - 1;
			}
		}
	}
}

void console__gotomap(Coordinate pos) {
	console__gotoxy(console__map2console(pos));
}

void console__gotoxy(Coordinate pos) {
#if defined _WIN32
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
#elif defined __linux
	wprintf(L"\e[%d;%df", pos.Y+1, pos.X+1);
	fflush(stdout);
#endif
}

void console__set_color_of(Coordinate pos) {
	Tile tile = engine__get_tile(pos);

	if(engine__is_same_coordinate(selection, pos))
		console__set_color(tile_shape_table[tile].color, MAGENTA);
	else if(engine__is_same_coordinate(cursor, pos))
		console__set_color(tile_shape_table[tile].color, CYAN);
	else
		console__set_color(tile_shape_table[tile].color, BLACK);
}

void console__set_color(COLOR fg, COLOR bg) {
	//fprintf(log_file, "[%s] fg:%d, bg:%d\n", __func__ , fg, bg); fflush(log_file);
#if defined _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), bg * 16 + fg);
#elif defined __linux
	wprintf(L"\e[1;3%dm\e[1;4%dm", fg, bg);
	fflush(stdout);
#endif
}

/**
 * arrow keys and enter key in windows and linux
 *
 *        | windows | linux
 * -------+---------+-----------
 *  up    | -32 72  | \e '[' 'A'
 *  down  | -32 72  | \e '[' 'B'
 *  right | -32 80  | \e '[' 'C'
 *  left  | -32 75  | \e '[' 'D'
 *  enter | '\r'    | '\n'
 *
 */
KEY console__get_selection_key() {
	char ch;
	static int arrow_count = 0;
	KEY ret = UNDEFINED;

	fprintf(log_file, "[%s] start\n", __func__); fflush(log_file);

	while ((ret == UNDEFINED) && console__kbhit()) {
		ch = console__get_key();
		fprintf(log_file, "[%s] key:%d\n", __func__, ch); fflush(log_file);
		switch(arrow_count) {
#if defined _WIN32
			case 0:
			switch(ch) {
				case 'w': case 'W':  ret = UP;     break;
				case 's': case 'S':  ret = DOWN;   break;
				case 'd': case 'D':  ret = RIGHT;  break;
				case 'a': case 'A':  ret = LEFT;   break;
				case 'p': case 'P':  ret = PAUSE;  break;
				case 'o': case 'O':  ret = AUTO;   break;
				case ' ': case '\r': ret = SELECT; break;
				case -32: arrow_count = 1;         break;
			}
			break;

			case 1:
			arrow_count = 0;
			switch (ch) {
				case 72: ret = UP;    break;
				case 80: ret = DOWN;  break;
				case 77: ret = RIGHT; break;
				case 75: ret = LEFT;  break;
			}
			break;
#elif defined __linux
			case 0:
			switch(ch) {
				case 'w': case 'W':  ret = UP;     break;
				case 's': case 'S':  ret = DOWN;   break;
				case 'd': case 'D':  ret = RIGHT;  break;
				case 'a': case 'A':  ret = LEFT;   break;
				case 'p': case 'P':  ret = PAUSE;  break;
				case 'o': case 'O':  ret = AUTO;   break;
				case ' ': case '\n': ret = SELECT; break;
				case '\e': arrow_count = 1;        break;
			}
			break;

			case 1: arrow_count = ch == '[' ? 2 : 0; break;

			case 2:
			arrow_count = 0;
			switch (ch) {
				case 'A': ret = UP;    break;
				case 'B': ret = DOWN;  break;
				case 'C': ret = RIGHT; break;
				case 'D': ret = LEFT;  break;
			}
			break;
#endif
		}
		fprintf(log_file, "[%s] count:%d\n", __func__, arrow_count); fflush(log_file);
	}

	fprintf(log_file, "[%s] end\n", __func__); fflush(log_file);
	return ret;
}

char console__get_key() {
#if defined _WIN32
	return getch();
#elif defined __linux
	struct termios old, new;
	char ch;

	tcgetattr(fileno(stdin), &old);
	new = old;

	new.c_lflag &= ~ICANON; // disable to print line by line
	new.c_lflag &= ~ECHO;

	tcsetattr(fileno(stdin), TCSANOW, &new);
	read(fileno(stdin), &ch, 1);
	tcsetattr(fileno(stdin), TCSANOW, &old);

	return ch;
#endif
}

bool console__kbhit() {
#if defined _WIN32
	return kbhit();
#elif defined __linux
	int byteswaiting;
	struct termios old, new;

	tcgetattr(fileno(stdin), &old);
	new = old;

	new.c_lflag &= ~ICANON; // disable to print line by line
	new.c_lflag &= ~ECHO;

	tcsetattr(fileno(stdin), TCSANOW, &new);
	ioctl(fileno(stdin), FIONREAD, &byteswaiting);
	tcsetattr(fileno(stdin), TCSANOW, &old);

	if(byteswaiting != 0) {
		fprintf(log_file, "[%s] %d\n", __func__, byteswaiting); fflush(log_file);
	}

	return byteswaiting > 0;
#endif
}

void console__set_echo(bool echo) {
#if defined __linux
	struct termios term;
	tcgetattr(fileno(stdin), &term);

	if(echo)
		term.c_lflag |=  ECHO;
	else
		term.c_lflag &= ~ECHO;

	tcsetattr(fileno(stdin), TCSANOW, &term);
#endif
}

void console__set_cursor_visibility(bool visibility) {
	if(visibility) {
#if defined _WIN32
		CONSOLE_CURSOR_INFO info;
		info.dwSize = 100;
		info.bVisible = TRUE;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
#elif defined __linux
		wprintf(L"\e[?25h");
#endif
	} else {
#if defined _WIN32
		CONSOLE_CURSOR_INFO info;
		info.dwSize = 100;
		info.bVisible = FALSE;
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
#elif defined __linux
		wprintf(L"\e[?25l");
#endif
	}
}

void console__clear() {
#if defined _WIN32
	system("cls");
	console__set_cursor_visibility(false);
#elif defined __linux
	system("clear");
#endif
}

void console__sleep(int ms) {
#if defined _WIN32
	Sleep(ms);
#elif defined __linux
	usleep(ms * 1000);
#endif
}

Coordinate console__map2console(Coordinate pos) {
#if defined _WIN32
	pos.X *= 3;
#elif defined __linux
	pos.X *= 2;
#endif
	return pos;
}
