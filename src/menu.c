#include <stdio.h>

#include "console.h"
#include "engine.h"
#include "menu.h"

#define BUFSIZE 1024

extern int map_row;
extern int map_col;
extern int score;
int max_hint = 5;
bool block_map_boundary = true;

extern FILE *log_file;

void menu__main() {
	bool gameover, playmore;
	bool init;
	wchar_t buf[BUFSIZE];
	wchar_t desc[]          = L"[ majang solitaire ]";
	wchar_t help_desc[]     = L"[ help ]\nscore is calculated as\n  100 * (deleted pair)\n- 10 * (failed attempts to delete)\n- (passed seconds)\n+ (clear bonus 1000 or 0)";
	const wchar_t *opts[]   = {L"new game", L"load game", L"config", L"help", L"exit", NULL};
	const wchar_t *yes_no[] = {L"yes", L"no", NULL};
	const wchar_t *ok[]     = {L"OK", NULL};

	while(true) {
		init = true;
		playmore = true;
		switch (menu__select_options(desc, opts)) {
			// load game, if success, just play as new game after load
			case 1:
				if(engine__load_game()) {
					init = false;
				} else {
					console__clear();
					console__set_echo(true);
					wprintf(L"failed to load game\n");
					wprintf(L"there is no save file or it was broken\n");
					console__sleep(1000);
					console__set_echo(false);
					break;
				}

			// new game
			case 0:
				while(playmore) {
					playmore = false;
					if(init) engine__init_game();
					gameover = engine__main();
					if(gameover) {
						swprintf(buf, BUFSIZE, L"your score is %d, would you like to play a new game?", score);
						if(menu__select_options(buf, yes_no) == 0)
							playmore = true;
						init = true;
					}
				}
			break;

			// config
			case 2: menu__config(); break;

			// help
			case 3: menu__select_options(help_desc, ok); break;

			// exit
			case 4: return; break;
		}
	}
}

void menu__config() {
	int input, min, max;
	wchar_t desc[BUFSIZE];
	const wchar_t *yes_no[] = {L"yes", L"no", NULL};
	wchar_t opt_row[BUFSIZE], opt_col[BUFSIZE], opt_hint[BUFSIZE], opt_block[BUFSIZE];
	const wchar_t *opts[] = {opt_row, opt_col, opt_hint, opt_block, L"exit", NULL};

	fprintf(log_file, "[%s] start function\n", __func__); fflush(log_file);

	while(true) {
		fprintf(log_file, "[%s] start loop\n", __func__); fflush(log_file);

		swprintf(desc,    BUFSIZE, L"[select option]\nrange of (row size)*(col size) is %d ~ %d", 2*NUM_TILE_SUIT, 2*MAX_PAIR*NUM_TILE_SUIT+1);
		swprintf(opt_row, BUFSIZE, L"set row size(now:%d)", map_row);
		swprintf(opt_col, BUFSIZE, L"set col size(now:%d)", map_col);
		if(max_hint == -1)
			swprintf(opt_hint, BUFSIZE, L"set hints (now:infinite)");
		else
			swprintf(opt_hint, BUFSIZE, L"set hints (now:%d)", max_hint);
		swprintf(opt_block, BUFSIZE, L"set block boundary mode(now:%ls)", block_map_boundary ? L"true" : L"false");

		fprintf(log_file, "[%s] start switch\n", __func__); fflush(log_file);
		switch (menu__select_options(desc, opts)) {
			case 0: // set row
				console__set_echo(true);

				min = (2*NUM_TILE_SUIT-1) / map_col + 1;
				max = (2*MAX_PAIR*NUM_TILE_SUIT+1) / map_col;
				if(max > MAX_MAP_ROW) max = MAX_MAP_ROW;

				console__clear();
				wprintf(L"[set row size (%d ~ %d)]\n", min, max);
				wprintf(L"input row size : ");
				scanf("%d", &input);

				if(input < min) input = min;
				if(input > max) input = max;
				map_row = input;

				console__set_echo(false);
			break;

			case 1: // set col
				console__set_echo(true);

				min = (2*NUM_TILE_SUIT-1) / map_row + 1;
				max = (2*MAX_PAIR*NUM_TILE_SUIT+1) / map_row;
				if(max > MAX_MAP_COL) max = MAX_MAP_COL;

				console__clear();
				wprintf(L"[set col size (%d ~ %d)]\n", min, max);
				wprintf(L"input col size : ");
				scanf("%d", &input);

				if(input < min) input = min;
				if(input > max) input = max;
				map_col = input;

				console__set_echo(false);
			break;

			case 2: // set hint
				console__set_echo(true);

				console__clear();
				wprintf(L"[set max hint usage]\n");
				wprintf(L"input hint usage (-1 is infinite, max is 100) : ");
				scanf("%d", &input);

				if(input < 0) input = -1;
				if(input > 100) input = 100;
				max_hint = input;

				console__set_echo(false);
			break;

			case 3: // set boundary crash
				swprintf(desc, BUFSIZE, L"block boundary when cursor crashed to the wall?");
				switch(menu__select_options(desc, yes_no)) {
					case 0: block_map_boundary = true;  break;
					case 1: block_map_boundary = false; break;
				}
			break;

			case 4: // exit
				return;
			break;
		}
	}

}


/**
 * return true when exit
 * return false when resume
 */
bool menu__pause() {
	wchar_t desc[]          = L"[ pause ]";
	const wchar_t *opts[]   = {L"resume", L"save game", L"exit", NULL};
	wchar_t exit_desc[]     = L"do you want to exit?";
	const wchar_t *yes_no[] = {L"yes", L"no", NULL};

	switch (menu__select_options(desc, opts)) {
		case 0: // resume
			return false;
		break;

		case 1: // save game
			engine__save_game();
			if(menu__select_options(exit_desc, yes_no) == 0)
				return true; // yes
			else
				return false; // no
		break;

		case 2: // exit
			return true;
		break;

		default:
			return false;
		break;
	}
}

/**
 * desc : description of options
 * opts : array of options, last must be a NULL
 * return : index of opts selected
 */
int menu__select_options(const wchar_t *desc, const wchar_t *opts[]) {
	int current_line = 0;
	int num_opts;
	int ret = -1;

	for(num_opts=0; opts[num_opts]; num_opts++);
	menu__print_options(desc, opts, num_opts, current_line);

	while(ret == -1) {
		if(console__kbhit()){
			fprintf(log_file, "[%s] kbhit\n", __func__); fflush(log_file);

			switch(console__get_selection_key()) {
				case UP:
					fprintf(log_file, "[%s] up\n", __func__); fflush(log_file);
					current_line = current_line > 0 ? current_line - 1 : 0;
				break;

				case DOWN:
					fprintf(log_file, "[%s] down\n", __func__); fflush(log_file);
					current_line = current_line < num_opts - 1 ? current_line + 1 : num_opts - 1;
				break;

				case SELECT: ret = current_line; break;

				default: break;
			}

			fprintf(log_file, "[%s] line:%d\n", __func__, current_line); fflush(log_file);
			menu__print_options(desc, opts, num_opts, current_line);
		}
	}

	console__clear();
	return ret;
}

void menu__print_options(const wchar_t *desc, const wchar_t *opts[], int num_opts, int line) {
	int i;

	console__set_color(WHITE, BLACK);
	console__set_echo(true);
	console__clear();
	wprintf(L"move : arrow key or wasd\tselect : space or enter key\n\n");
	wprintf(L"%ls\n", desc);
	for(i=0; i<num_opts; i++)
		wprintf(L"%ls %ls\n", i == line ? L">" : L" ", opts[i]);

	console__set_echo(false);
}
