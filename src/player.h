#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>

#include "SDL2/SDL.h"

#include "map.h"
#include "utils.h"

#define ANIMATION_SPEED 100 //MS
#define DEATH_ANIMATION_SPEED 250 //MS

struct Player;
typedef struct Player Player;

Player *player_load(SDL_Renderer *renderer);
void player_free(Player *player);

void player_reset(Player *player);
void player_input(Player *player, SDL_Event *e);
void player_update(Player *player, int delta_time, Map *map, float min_x, float max_x);
void player_draw(Player *player, SDL_Renderer *renderer, SDL_Point *camera_offset);

void player_kill(Player *player);
void player_play_death_animation(Player *player, int delta_time);
const SDL_FPoint *player_get_pos(Player *player);
const SDL_FRect player_get_box(Player *player);
SDL_Texture *player_get_texture(Player *player);

#endif