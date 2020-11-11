#include "ghost.h"

#include <math.h>
#include <stdio.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "a_star.h"
#include "debug.h"
#include "utils.h"

struct Ghost {
	SDL_Texture *texture;
	SDL_FPoint starting_position;

	SDL_FPoint position;

	Direction current_direction;

	SDL_Point *path;
	int path_length;
	int update_path_timer;
	float current_position_in_path;

	SDL_Rect sprite;

	GhostState state;
	int initial_wait_time;
	int exit_timer;

	float speed;
};

void ghost_reset(Ghost *this, const float speed) {
	this->position = this->starting_position;

	this->current_direction = NORTH;

	// Pathfinding
	this->path = NULL;
	this->path_length = 0;
	this->current_position_in_path = 0.0f;

	this->state = WAITING;
	this->speed = speed;
	this->exit_timer = this->initial_wait_time;
}

void ghost_switch_state(Ghost *this, const GhostState state) {
	if (this->state == state)
		return;
	this->state = state;

	switch (state) {
		case FLEEING: {
			this->update_path_timer = 0;
		} break;
		case ATTACKING: {
			this->update_path_timer = 0;
		} break;
	}
}

SDL_FPoint *ghost_get_pos(Ghost *this) {
	return &this->position;
}

Ghost *create_ghost(SDL_Renderer *renderer, const float x, const float y, const int wait_time, const int sprite_x, const int sprite_y) {
	Ghost *this = malloc(sizeof(Ghost));

	SDL_Surface *surf = IMG_Load("resources/ghost.png");
	this->texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);

	this->sprite.x = sprite_x;
	this->sprite.y = sprite_y;
	this->sprite.w = 16;
	this->sprite.h = 16;

	this->starting_position.x = x;
	this->starting_position.y = y;
	this->initial_wait_time = wait_time;
	this->exit_timer = wait_time;

	return this;
}

void destroy_ghost(Ghost *ghost) {
	SDL_DestroyTexture(ghost->texture);
	free(ghost->path);
	free(ghost);
}

static void update_path(Ghost *this, const SDL_FPoint *player_pos, Map *map) {
	SDL_Point a = { round(this->position.x), round(this->position.y) };
	SDL_Point b = { (int)player_pos->x, (int)player_pos->y };
	a_star(map, &a, &b, &this->path, &this->path_length);
	this->current_position_in_path = 0;
}

static void update_flee_path(Ghost *this, const SDL_FPoint *player_pos, Map *map) {
	SDL_Point a = { round(this->position.x), round(this->position.y) };
	SDL_Point b = { (int)player_pos->x, (int)player_pos->y };
	reverse_a_star(map, &a, &b, 4, &this->path, &this->path_length);
	this->current_position_in_path = 0;
}

void move_ghost(Ghost *this, int delta_time, Map *map) {
	if (this->path_length <= 0)
		return;

	SDL_FPoint dir = {
		this->path[(int)this->current_position_in_path + 1].x - this->position.x,
		this->path[(int)this->current_position_in_path + 1].y - this->position.y
	};

	SDL_FPoint_Normalize(&dir);

	float speed = this->speed * delta_time / 1000.0f;
	SDL_FPoint new_pos = this->position;

	new_pos.x = this->position.x + speed * dir.x;
	new_pos.y = this->position.y + speed * dir.y;

	this->position = new_pos;

	if (SDL_1FPoint_Distance(&this->path[(int)this->current_position_in_path + 1], &this->position) <= speed)
		this->current_position_in_path++;
}

void update_ghost(Ghost *this, int delta_time, const SDL_FPoint *player_pos, Map *map) {
	switch (this->state) {
		case WAITING: {
			this->exit_timer -= delta_time;
			if (this->position.y <= 13.0f) {
				this->current_direction = SOUTH;
			} else if (this->position.y >= 15.0f) {
				this->current_direction = NORTH;
			}
			if (this->current_direction == NORTH)
				this->position.y -= this->speed * delta_time / 1000.0f;
			else if (this->current_direction == SOUTH)
				this->position.y += this->speed * delta_time / 1000.0f;
			if (this->exit_timer <= 0)
				this->state = ATTACKING;
		} break;
		case ATTACKING: {
			this->update_path_timer -= delta_time;
			if (this->update_path_timer <= 0 || this->current_position_in_path + 1 >= this->path_length) {
				this->update_path_timer = PATH_UPDATE_FREQ;
				update_path(this, player_pos, map);
			}
			move_ghost(this, delta_time, map);
		} break;
		case FLEEING: {
			this->update_path_timer -= delta_time;
			if (this->update_path_timer <= 0 || this->current_position_in_path + 1 >= this->path_length) {
				this->update_path_timer = PATH_UPDATE_FREQ;
				update_flee_path(this, player_pos, map);
			}
			move_ghost(this, delta_time, map);
		} break;
		case DEAD: {
			this->update_path_timer -= delta_time;
			if (this->update_path_timer <= 0) {
				this->update_path_timer = PATH_UPDATE_FREQ;
				update_path(this, &this->starting_position, map);
			}
			move_ghost(this, delta_time, map);
			if (this->current_position_in_path + 1 >= this->path_length) {
				ghost_switch_state(this, ATTACKING);
			}
		} break;
	}
}

void ghost_kill(Ghost *this) {
	this->state = DEAD;
	this->update_path_timer = 0;
}

void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset) {
	SDL_Rect dst = { camera_offset->x + (ghost->position.x) * 16, camera_offset->y + (ghost->position.y) * 16, 16, 16 };
	SDL_Rect src = ghost->sprite;
	if (ghost->state == DEAD || ghost->state == FLEEING) {
		src.x = 32;
		src.y = 0;
	}

	SDL_RenderCopy(renderer, ghost->texture, &src, &dst);
}

void dbg_draw_ghost(Ghost *this, SDL_Renderer *renderer, TTF_Font *font, const SDL_Point *camera_offset) {
	for (int i = 0; i < this->path_length; i++) {
		Uint8 r = (255 / this->path_length * i);

		SDL_SetRenderDrawColor(renderer, r, 255, 255, 255);
		if (this->state == FLEEING)
			SDL_SetRenderDrawColor(renderer, r, 0, 255, 255);

		SDL_Rect dst = { this->path[i].x * 16 + camera_offset->x, this->path[i].y * 16 + camera_offset->y, 16, 16 };
		SDL_RenderDrawRect(renderer, &dst);
	}
	if (this->path != NULL) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		SDL_Rect dst = { this->path[(int)this->current_position_in_path + 1].x * 16 + camera_offset->x, this->path[(int)this->current_position_in_path + 1].y * 16 + camera_offset->y, 16, 16 };
		//SDL_RenderDrawRect(renderer, &dst);

		SDL_RenderDrawLine(renderer, this->position.x * 16 + camera_offset->x, this->position.y * 16 + camera_offset->y, this->path[(int)this->current_position_in_path + 1].x * 16 + camera_offset->x, this->path[(int)this->current_position_in_path + 1].y * 16 + camera_offset->y);
	}
}

void ghost_change_state(Ghost *this, const GhostState state) {
	if (this->state != DEAD) {
		this->state = state;
		switch (state) {
			case FLEEING: {
				this->speed /= 2;
			}
			case ATTACKING: {
				this->speed *= 2;
			}
		}
	}
}