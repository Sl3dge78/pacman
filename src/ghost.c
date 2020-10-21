#include "ghost.h"

#include <math.h>
#include <stdio.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "a_star.h"
#include "debug.h"
#include "utils.h"

enum GhostState {
	WAITING,
	ATTACKING,
	FLEEING
} typedef GhostState;

struct Ghost {
	SDL_Texture *texture;
	SDL_FPoint position;
	Direction current_direction;
	SDL_Point current_destination;

	SDL_Point *path;
	int path_length;
	int update_path_timer;
	int current_position_in_path;

	SDL_Rect src;

	GhostState state;
};

void ghost_reset(Ghost *this) {
	this->position.x = 1.0f;
	this->position.y = 1.0f;
	this->current_direction = NORTH;

	// Pathfinding
	this->path = NULL;
	this->path_length = 0;
	this->current_position_in_path = 0;
	this->current_destination.x = (int)this->position.x;
	this->current_destination.y = (int)this->position.y;

	this->src.x = 0;
	this->src.y = 0;
	this->src.w = 16;
	this->src.h = 16;

	this->state = WAITING;
}

SDL_FPoint *ghost_get_pos(Ghost *this) {
	return &this->position;
}

Ghost *create_ghost(SDL_Renderer *renderer) {
	Ghost *this = malloc(sizeof(Ghost));

	SDL_Surface *surf = IMG_Load("resources/ghost.png");
	this->texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);

	ghost_reset(this);
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
	this->current_destination.x = this->position.x;
	this->current_destination.y = this->position.y;
}

static void next_node(Ghost *this) {
	this->current_position_in_path++;
	if (this->current_position_in_path >= this->path_length) {
		this->current_destination.x = this->position.x;
		this->current_destination.y = this->position.y;
		return;
	}

	this->current_destination = this->path[this->current_position_in_path];

	if (this->current_destination.x < this->position.x && this->current_destination.y == (int)this->position.y)
		this->current_direction = WEST;
	else if (this->current_destination.x > this->position.x && this->current_destination.y == (int)this->position.y)
		this->current_direction = EAST;
	else if (this->current_destination.x == (int)this->position.x && this->current_destination.y < this->position.y)
		this->current_direction = NORTH;
	else if (this->current_destination.x == (int)this->position.x && this->current_destination.y > this->position.y)
		this->current_direction = SOUTH;
}

void update_ghost(Ghost *this, int delta_time, const SDL_FPoint *player_pos, Map *map) {
	this->update_path_timer -= delta_time;
	if (this->update_path_timer <= 0) {
		this->update_path_timer = PATH_UPDATE_FREQ;
		update_path(this, player_pos, map);
		next_node(this);
	}

	if (this->path_length == this->current_position_in_path || this->path == NULL) {
		// We reached destination
		update_path(this, player_pos, map);
		next_node(this);
	}

	switch (this->current_direction) {
		case NORTH:
			this->position.y -= GHOST_SPEED * delta_time / 1000;
			this->position.x = round(this->position.x);
			if (this->position.y < this->current_destination.y)
				next_node(this);
			break;
		case SOUTH:
			this->position.y += GHOST_SPEED * delta_time / 1000;
			this->position.x = round(this->position.x);
			if (this->position.y > this->current_destination.y)
				next_node(this);
			break;
		case WEST:
			this->position.x -= GHOST_SPEED * delta_time / 1000;
			this->position.y = round(this->position.y);
			if (this->position.x < this->current_destination.x)
				next_node(this);
			break;
		case EAST:
			this->position.x += GHOST_SPEED * delta_time / 1000;
			this->position.y = round(this->position.y);
			if (this->position.x > this->current_destination.x)
				next_node(this);
			break;
	}
}

void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset) {
	SDL_Rect dst = { camera_offset->x + ghost->position.x * 16, camera_offset->y + ghost->position.y * 16, 16, 16 };
	SDL_RenderCopy(renderer, ghost->texture, &ghost->src, &dst);
}

void dbg_draw_ghost(Ghost *this, SDL_Renderer *renderer, TTF_Font *font, const SDL_Point *camera_offset) {
	char buffer[20];
	sprintf(buffer, "%f %f - %d", this->position.x, this->position.y, this->current_direction);
	SDL_Point dst = { 0, 16 };
	draw_text(renderer, font, buffer, &dst, ALIGN_LEFT);

	for (int i = 0; i < this->path_length; i++) {
		Uint8 r = 255 - ((this->path_length - i) * 255);
		SDL_SetRenderDrawColor(renderer, r, 255, 255, 255);
		SDL_Rect dst = { this->path[i].x * 16 + camera_offset->x, this->path[i].y * 16 + camera_offset->y, 16, 16 };
		SDL_RenderDrawRect(renderer, &dst);
	}
}