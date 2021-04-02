#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "console.h"
#include "engine.h"
#include "menu.h"
#include "util.h"

typedef struct {
	int num_tile;
	Coordinate pos[MAX_PAIR * 2];
} Located_tile;

typedef struct {
	clock_t passed_clock;
	int score;
	int left_hint;
	int map_row;
	int map_col;
} File_header;

typedef struct {
	int tile;
	int x;
	int y;
} File_block;

const char save_file_name[] = "save.dat";
extern FILE *log_file;

Tile map[MAX_MAP_ROW][MAX_MAP_COL];
Located_tile located_tile_table[NUM_TILE_SUIT];

Coordinate cursor = {1, 1}, selection = {-1, -1};

int passed_second;
int score;
int left_hint;
clock_t passed_clock, start_clock;

int map_row = MAX_MAP_ROW/2;
int map_col = MAX_MAP_COL/2;
extern int max_hint;


/**
 * return true when game is over
 * return false when game is paused
 */
bool engine__main() {
	KEY key;
	int num_point;
	bool is_solvable;
	Coordinate pair[2], path[4];

	console__print_map(passed_second, score, left_hint);
	is_solvable = engine__find_removable_pair(pair);
	fprintf(log_file, "[%s] get new pair\n", __func__);

	while(is_solvable) {
		// if user hit the keyboard handle it
		if(console__kbhit()) {
			switch(key = console__get_selection_key()) {
				case UP: case DOWN: case RIGHT: case LEFT:
					fprintf(log_file, "[%s] key:arrow\n", __func__); fflush(log_file);
					console__move(key);
				break;

				case SELECT:
					fprintf(log_file, "[%s] key:select\n", __func__); fflush(log_file);
					if(engine__is_tile_empty(cursor)) break;

					if(selection.X == -1 && selection.Y == -1) {
						selection = cursor;
					} else if(engine__is_same_coordinate(cursor, selection)) {
						selection.X = selection.Y = -1; // deselect
					} else {
						pair[0] = cursor; pair[1] = selection;
						selection.X = selection.Y = -1; // deselect

						if(engine__is_same_tile(pair[0], pair[1]) && (num_point = engine__get_path(pair, path)) > 0) {
							console__flash_path(path, num_point);
							engine__delete_tile(pair[0]);
							engine__delete_tile(pair[1]);
							fprintf(log_file, "[%s] get new pair\n", __func__); fflush(log_file);
							score += 100;
						} else {
							score -= 10;
						}

						console__update_score(score);
						console__set_color_of(pair[1]);
						console__print_tile(pair[1]);
						is_solvable = engine__find_removable_pair(pair);
					}

					console__set_color_of(cursor);
					console__print_tile(cursor);
					fprintf(log_file, "[%s] selection:(%d,%d)\n", __func__, selection.X, selection.Y); fflush(log_file);
				break;

				case AUTO:
					fprintf(log_file, "[%s] key:auto\n", __func__); fflush(log_file);
					// if hint is left or infinite
					if(left_hint > 0 || left_hint == -1) {
						num_point = engine__get_path(pair, path);
						assert(num_point > 0, "failed to get path");

						console__flash_path(path, num_point);
						engine__delete_tile(pair[0]);
						engine__delete_tile(pair[1]);

						is_solvable = engine__find_removable_pair(pair);
						fprintf(log_file, "[%s] get new pair\n", __func__);

						if(left_hint != -1) {
							left_hint--;
							console__update_hint(left_hint);
						}
					}
				break;

				case PAUSE:
					fprintf(log_file, "[%s] key:pause\n", __func__); fflush(log_file);

					passed_clock += clock() - start_clock;
					if(menu__pause()) return false;
					start_clock = clock();

					console__clear();
					console__print_map(passed_second, score, left_hint);
				break;

				default: break;
			}
		}

		// if time is passed more then 1 second from latest updated time
		if((passed_clock + (clock()-start_clock)) / CLOCKS_PER_SEC > passed_second) {
			passed_second++;
			console__update_time(passed_second);
		}
	}
	// end of loop

	score -= passed_second;
	if(engine__is_map_empty()) score += 1000;

	return true;
}

void engine__init_game() {
	engine__choose_tile();
	engine__shuffle_map();
	assert(engine__set_located_tile_table(), "error on engine__set_located_tile_table");

	cursor.X = cursor.Y = 1;
	selection.X = selection.Y = -1;
	passed_second = 0;
	passed_clock = 0;
	start_clock = clock();
	left_hint = max_hint;
	score = 0;
}

void engine__save_game() {
	int x, y;
	File_header header;
	File_block block;
	FILE *f = fopen(save_file_name, "wb");
	assert(f!=NULL, "save wile open w");


	header.passed_clock = passed_clock;
	header.score = score;
	header.left_hint = left_hint;
	header.map_row = map_row;
	header.map_col = map_col;

	fwrite(&header, sizeof(header), 1, f);

	for(x=0; x<map_row; x++) {
		for(y=0; y<map_col; y++) {
			if(map[x][y] != -1) {
				block.x = x+1;
				block.y = y+1;
				block.tile = map[x][y] + 1;
				fwrite(&block, sizeof(block), 1, f);
			}
		}
	}

	fclose(f);
}

bool engine__load_game() {
	int x, y;
	File_header header;
	File_block block;
	FILE *f = fopen(save_file_name, "rb");
	if(!f) return false;

	fread(&header, sizeof(header), 1, f);

	passed_clock = header.passed_clock;
	score = header.score;
	left_hint = header.left_hint;
	map_row = header.map_row;
	map_col = header.map_col;
	passed_second = passed_clock / CLOCKS_PER_SEC;

	fprintf(log_file, "[%s] passed_clock:%d\n", __func__, (int)passed_clock); fflush(log_file);
	fprintf(log_file, "[%s] score:%d\n", __func__, score); fflush(log_file);
	fprintf(log_file, "[%s] left_hint:%d\n", __func__, left_hint); fflush(log_file);
	fprintf(log_file, "[%s] map_row:%d\n", __func__, map_row); fflush(log_file);
	fprintf(log_file, "[%s] map_col:%d\n", __func__, map_col); fflush(log_file);
	fprintf(log_file, "[%s] passed_second:%d\n", __func__, passed_second); fflush(log_file);

	if(passed_clock < 0) return false;
	if(passed_second < 0) return false;
	if(left_hint < -1) return false;
	if( ! engine__is_vaild_mapsize()) return false;

	// set all tile to empty
	for(x=0; x<map_row; x++) {
		for(y=0; y<map_col; y++) {
			map[x][y] = -1;
		}
	}

	while(fread(&block, sizeof(block), 1, f)) {
		fprintf(log_file, "[%s] tile:%d x:%d y:%d\n", __func__, block.tile, block.x, block.y); fflush(log_file);
		if(block.x < 1 || map_row < block.x) return false;
		if(block.y < 1 || map_col < block.y) return false;
		if(map[block.x-1][block.y-1] != -1) continue;
		if(block.tile < 1 || NUM_TILE_SUIT < block.tile) continue;

		map[block.x-1][block.y-1] = block.tile-1;
	}

	fclose(f);

	return engine__set_located_tile_table();
}

/**
 * select tiles by shuffling tiles
 */
void engine__choose_tile() {
	int i, x, y, random;
	Tile temp, tiles[MAX_PAIR*NUM_TILE_SUIT];

	assert(engine__is_vaild_mapsize(), "wrong map size");

	// initialize
	for(i=0; i<MAX_PAIR*NUM_TILE_SUIT; i++) {
		tiles[i] = i % NUM_TILE_SUIT;
		fprintf(log_file, "[%s] i:%d, tiles[i]:%d\n", __func__, i, tiles[i]); fflush(log_file);
	}

	// do not shuffle 0 ~ NUM_TILE_SUIT
	// every suit should choosed into map
	for(i=MAX_PAIR*NUM_TILE_SUIT-1; i>NUM_TILE_SUIT; i--) {
		random = engine__fair_random(NUM_TILE_SUIT, i);
		assert((NUM_TILE_SUIT <= random && random <= i), "fair random error");
		fprintf(log_file, "[%s] random:%d\n", __func__, random); fflush(log_file);
		temp = tiles[i];
		tiles[i] = tiles[random];
		tiles[random] = temp;
	}

	for(i=0; i<MAX_PAIR*NUM_TILE_SUIT; i++) {
		fprintf(log_file, "[%s] tiles[%d]:%d\n", __func__, i, tiles[i]); fflush(log_file);
	}

	// insert pairs into map
	for(i=0; i<(map_row*map_col/2)*2; i++) {
		x = i % map_row;
		y = i / map_row;
		map[x][y] = tiles[i/2];
		fprintf(log_file, "[%s] map[%d][%d]:%d\n", __func__, x, y, map[x][y]); fflush(log_file);
	}

	// if size of map is odd, last one is empty tile
	if((map_row*map_col) % 2 == 1)
		map[map_row-1][map_col-1] = -1;

}

void engine__shuffle_map() {
	if(map == NULL) return;
	int i, x, y, randx, randy, random;
	Tile temp;

	for(i=map_row*map_col-1; i>0; i--) {
		random = engine__fair_random(0, i);
		assert((0 <= random && random <= i), "fair random error");

		x = i % map_row;
		y = i / map_row;
		randx = random % map_row;
		randy = random / map_row;

		fprintf(log_file, "[%s] i:%d, x:%d, y:%d, randx:%d, randy:%d\n", __func__, i, x, y, randx, randy); fflush(log_file);

		temp = map[x][y];
		map[x][y] = map[randx][randy];
		map[randx][randy] = temp;
	}

	for(i=0; i<map_row*map_col; i++) {
		x = i % map_row;
		y = i / map_row;

		fprintf(log_file, "[%s] map[%d][%d]:%d\n", __func__, x, y, map[x][y]); fflush(log_file);
	}
}

bool engine__set_located_tile_table() {
	int i, x, y, num_tile;
	Tile tile;
	Coordinate pos;

	for(i=0; i<NUM_TILE_SUIT; i++)
		located_tile_table[i].num_tile = 0;

	for(i=0; i<map_row*map_col; i++) {
		x = i % map_row;
		y = i / map_row;

		tile = map[x][y];
		if(tile == -1) continue;

		pos.X = x+1, pos.Y = y+1;
		num_tile = located_tile_table[tile].num_tile;
		fprintf(log_file, "[%s] i:%d, x:%d, y:%d, tile:%d, num_tile:%d\n", __func__, i, pos.X, pos.Y, tile, num_tile); fflush(log_file);

		if(num_tile > MAX_PAIR*2) return false;
		located_tile_table[tile].pos[num_tile] = pos;
		located_tile_table[tile].num_tile++;
	}

	for(i=0; i<NUM_TILE_SUIT; i++)
		if(located_tile_table[i].num_tile % 2 == 1) return false;

	return true;
}

bool engine__is_map_empty() {
	int x, y;

	for(x=0; x<map_row; x++)
		for(y=0; y<map_col; y++)
			if(map[x][y] != -1)
				return false;

	return true;
}

/**
 * in worse case, runs engine__get_path() NUM_TILE_SUIT * (2*MAX_PAIR-1) * MAX_PAIR times
 * thus, this runs O(NUM_TILE_SUIT * MAX_PAIR^2 * (map_row + map_col)^2) times
 */
bool engine__find_removable_pair(Coordinate pair[2]) {
	int i, j;
	Tile tile;
	Coordinate path[4];

	if(pair == NULL) return 0;

	for(tile=0; tile<NUM_TILE_SUIT; tile++) {
		for(i=0; i<located_tile_table[tile].num_tile; i++) {
			pair[0] = located_tile_table[tile].pos[i];
			if(engine__get_tile(pair[0]) == -1) continue;

			for(j=0; j<i; j++) {
				pair[1] = located_tile_table[tile].pos[j];
				if(engine__get_tile(pair[1]) == -1) continue;
				if(engine__get_path(pair, path)) return true;
			}
		}
	}

	return false;
}

/**
 * there are 6 cases in 3 segment path which are generalized into 2 big cases
 *
 * 1-* cases have horizontal segment in second segment
 * 2-* cases have vertical segment in second segment
 *
 *
 * 1-1            :   1-2           :   1-3
 * +---------+    :   o             :   o
 * |         |    :   |             :   |
 * |         |    :   |             :   |         o
 * o         |    :   +---------+   :   |         |
 *           |    :             |   :   |         |
 *           |    :             |   :   |         |
 *           o    :             o   :   +---------+
 *                :                 :
 * ===============#=================#===================
 *                :                 :
 * 2-1            :  2-2            :  2-3
 * +-----o        :  o-----+        :  o-----------+
 * |              :        |        :              |
 * |              :        |        :              |
 * |              :        |        :              |
 * +-----------o  :        +-----o  :        o-----+
 *
 *
 *
 * this function runs engine__is_available_path() O(map_row + map_col) times
 * thus, runs in O((map_row + map_col)^2) times
 *
 *
 * returns number of points of path(if 0, it failed to find path)
 */
int engine__get_path(const Coordinate pair[2], Coordinate path[4]) {
	int i;

	if(pair == NULL || path == NULL) return 0;
	if( ! engine__is_available_coordinate(pair[0])) return 0;
	if( ! engine__is_available_coordinate(pair[1])) return 0;

	path[0] = pair[0];

	/* 1 segment */

	//fprintf(log_file, "[%s] 2\n", __func__); fflush(log_file);
	path[1] = pair[1];
	if(engine__is_available_path(path, 2)) return 2;

	/* 2 segments */

	//fprintf(log_file, "[%s] 3\n", __func__); fflush(log_file);
	path[2] = pair[1];
	if(pair[0].X != pair[1].X && pair[0].Y != pair[1].Y) {
		//fprintf(log_file, "[%s] 3-1\n", __func__); fflush(log_file);
		path[1].X = pair[0].X;
		path[1].Y = pair[1].Y;
		if(engine__is_available_path(path, 3)) return 3;
		//fprintf(log_file, "[%s] 3-2\n", __func__); fflush(log_file);
		path[1].X = pair[1].X;
		path[1].Y = pair[0].Y;
		if(engine__is_available_path(path, 3)) return 3;
	}

	/* 3 segments */

	//fprintf(log_file, "[%s] 4\n", __func__); fflush(log_file);
	path[3] = pair[1];

	// case when second segment is horizontal
	if(pair[0].X != pair[1].X) {
		for(i=0; i<=map_col+1; i++) {
			if(i == pair[0].Y || i == pair[1].Y) continue;
			//fprintf(log_file, "[%s] 4-horizontal-%d\n", __func__, i); fflush(log_file);
			path[1].X = pair[0].X;
			path[1].Y = i;
			path[2].X = pair[1].X;
			path[2].Y = i;
			if(engine__is_available_path(path, 4)) return 4;
		}
	}

	// case when second segment is vertical
	if(pair[0].Y != pair[1].Y) {
		for(i=0; i<=map_row+1; i++) {
			if(i == pair[0].X || i == pair[1].X) continue;
			//fprintf(log_file, "[%s] 4-vertical-%d\n", __func__, i); fflush(log_file);
			path[1].X = i;
			path[1].Y = pair[0].Y;
			path[2].X = i;
			path[2].Y = pair[1].Y;
			if(engine__is_available_path(path, 4)) return 4;
		}
	}

	return 0;
}

/**
 * delete tile from map and located_tile_table
 */
void engine__delete_tile(Coordinate pos) {
	bool checked = false;
	int i, num_tile;
	Tile tile;

	if( ! engine__is_available_coordinate(pos)) return;
	if(engine__is_tile_empty(pos)) return;

	if(engine__is_same_coordinate(selection, pos)) {
		selection.X = selection.Y = -1;
	}

	// delete from map
	tile = map[pos.X-1][pos.Y-1];
	map[pos.X-1][pos.Y-1] = -1;

	fprintf(log_file, "[%s] x:%d, y:%d, tile:%d\n", __func__, pos.X, pos.Y, tile); fflush(log_file);

	// delete from located_tile_table
	for(i=0; i<located_tile_table[tile].num_tile; i++) {
		if(engine__is_same_coordinate(located_tile_table[tile].pos[i], pos)) {
			located_tile_table[tile].num_tile--;
			num_tile = located_tile_table[tile].num_tile;
			located_tile_table[tile].pos[i] = located_tile_table[tile].pos[num_tile];
			checked = true;
			break;
		}
	}

	assert(checked, "located_tile_table may broken");
	console__set_color_of(pos);
	console__print_tile(pos);
}

/**
 * each segment of path, we do not check last coordinate
 * so this function avoid only first coordinate of path
 *
 * this runs O(map_row + map_col) times
 */
bool engine__is_available_path(const Coordinate path[4], int num_point) {
	if(map == NULL || path == NULL || num_point < 2) return false;
	if( ! engine__is_available_coordinate(path[0])) return false;
	Coordinate pos;
	int i;

	for(i=1; i<num_point; i++) {
		if( ! engine__is_available_coordinate(path[i])) return false;
		pos = path[i-1];


		if(path[i-1].X == path[i].X) {
			while(pos.Y != path[i].Y) {
				//fprintf(log_file, "[%s] in %dth vertical segment - (%d,%d)\n", __func__, i, pos.X, pos.Y); fflush(log_file);
				// pass if is first coordinate
				if( ! (i == 1 && pos.Y == path[i-1].Y))
					if( ! engine__is_tile_empty(pos))
						return false;

				pos.Y = (path[i-1].Y < path[i].Y) ? pos.Y + 1 : pos.Y - 1;
			}
		}
		else if(path[i-1].Y == path[i].Y) {
			while(pos.X != path[i].X) {
				//fprintf(log_file, "[%s] in %dth horizontal segment - (%d,%d)\n", __func__, i, pos.X, pos.Y); fflush(log_file);
				// pass if is first coordinate
				if( ! (i == 1 && pos.X == path[i-1].X))
					if( ! engine__is_tile_empty(pos))
						return false;

				pos.X = (path[i-1].X < path[i].X) ? pos.X + 1 : pos.X - 1;
			}
		}
		else {
			return false;
		}
	}
	return true;
}

/**
 * is pos in range of map or beside of map
 */
bool engine__is_available_coordinate(Coordinate pos) {
	if(0 <= pos.X && pos.X <= map_row+1 && 0 <= pos.Y && pos.Y <= map_col+1)
		return true;
	else
		return false;
}

bool engine__is_tile_empty(Coordinate pos) {
	if(0 < pos.X && pos.X <= map_row && 0 < pos.Y && pos.Y <= map_col) {
		if(map[pos.X-1][pos.Y-1] == -1)
			return true;
		else
			return false;
	}

	return true;
}

bool engine__is_same_coordinate(Coordinate a, Coordinate b) {
	return a.X == b.X && a.Y == b.Y;
}

bool engine__is_same_tile(Coordinate a, Coordinate b) {
	return engine__get_tile(a) == engine__get_tile(b);
}

Tile engine__get_tile(Coordinate pos) {
	if(1 <= pos.X && pos.X <= map_row && 1 <= pos.Y && pos.Y <= map_col)
		return map[pos.X-1][pos.Y-1];
	else
		return -1;
}

/**
 * include from and to
 */
int engine__fair_random(int from, int to) {
	unsigned long long random;
	int diff = from<to ? to-from : from-to;

	do {
		random = rand();
	} while(random >= ((RAND_MAX+(long long)1) / diff+1) * diff+1);

	return (from<to ? from : to) + random % (diff+1);
}

bool engine__is_vaild_mapsize() {
	if( ! (1 <= map_row && map_row <= MAX_MAP_ROW)) return false;
	if( ! (1 <= map_col && map_col <= MAX_MAP_COL)) return false;
	if( ! (2*NUM_TILE_SUIT <= map_row*map_col)) return false;
	if( ! (map_row*map_col <= 2*MAX_PAIR*NUM_TILE_SUIT+1)) return false;
	return true;

}
