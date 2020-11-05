#ifndef MAP_H
#define MAP_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include "utils.h"

#define MAP_WIDTH 28
#define MAP_HEIGHT 31
#define MAP_SIZE MAP_WIDTH *MAP_HEIGHT

enum CollisionMask {
	COLLISION_PLAYER = 1,
	COLLISION_GHOST = 2,
} typedef CollisionMask;

enum Tile {
	POWERUP = -3,
	PAC = -2,
	EMPTY = -1,
	TURN_RIGHT = 0,
	TURN_DOWN = 1,
	STRAIGHT_HOR = 2,
	TURN_UP = 3,
	TURN_LEFT = 4,
	STRAIGHT_VER = 5,

} typedef Tile;

struct Map_;
typedef struct Map_ Map;

Map *map_load(SDL_Renderer *renderer);
void reset_map(Map *map);
void map_draw(Map *map, SDL_Renderer *renderer, SDL_Point *camera_offset);
void map_free(Map *map);

bool map_get_collision(const Map *map, const int x, const int y, const CollisionMask bitmask);
Tile map_eat_at(Map *map, const int x, const int y);
void map_toggle_color(Map *map);
void map_reset_color(Map *map);
#endif