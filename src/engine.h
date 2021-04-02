#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include "console.h"

#define MAX_MAP_ROW   30
#define MAX_MAP_COL   20
#define MAX_PAIR       5
#define NUM_TILE_SUIT 32

typedef int Tile;

bool engine__main();
void engine__init_game();
void engine__save_game();
bool engine__load_game();
void engine__choose_tile();
void engine__shuffle_map();
bool engine__set_located_tile_table();
bool engine__find_removable_pair(Coordinate pair[2]);
int  engine__get_path(const Coordinate pair[2], Coordinate path[4]);
void engine__delete_tile(Coordinate pos);
bool engine__is_map_empty();
bool engine__is_available_path(const Coordinate path[4], int num_point);
bool engine__is_available_coordinate(Coordinate pos);
bool engine__is_tile_empty(Coordinate pos);
bool engine__is_same_coordinate(Coordinate a, Coordinate b);
bool engine__is_same_tile(Coordinate a, Coordinate b);
Tile engine__get_tile(Coordinate pos);
int  engine__fair_random(int from, int to);
bool engine__is_vaild_mapsize();

#endif // ENGINE_H
