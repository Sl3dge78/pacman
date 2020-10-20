#include "ghost.h"

#include <stdio.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

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
	SDL_FPoint next_position;

	SDL_Point *path;
	int path_length;
	int update_path_timer;

	SDL_Rect src;

	GhostState state;
};

Ghost *create_ghost(SDL_Renderer *renderer) {
	Ghost *ghost = malloc(sizeof(Ghost));

	SDL_Surface *surf = IMG_Load("resources/ghost.png");
	ghost->texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);

	ghost->position.x = 1.0f;
	ghost->position.y = 1.0f;

	ghost->path_length = 0;
	ghost->path = NULL;

	ghost->src.x = 0;
	ghost->src.y = 0;
	ghost->src.w = 16;
	ghost->src.h = 16;

	ghost->state = WAITING;
}

void destroy_ghost(Ghost *ghost) {
	SDL_DestroyTexture(ghost->texture);
	free(ghost->path);
	free(ghost);
}

static void update_path(Ghost *ghost, const SDL_FPoint *player_pos, Map *map) {
	SDL_Point a = { (int)ghost->position.x, (int)ghost->position.y };
	SDL_Point b = { (int)player_pos->x, (int)player_pos->y };
	a_star(map, &a, &b, &ghost->path, &ghost->path_length);
}

void update_ghost(int delta_time, Ghost *ghost, const SDL_FPoint *player_pos, Map *map) {
	ghost->update_path_timer -= delta_time;

	SDL_Point dst = { 0, 0 };

	if (ghost->path_length > 1) {
		dst.x = ghost->path[1].x;
		dst.y = ghost->path[1].y;
	}
	SDL_Point org = { (int)ghost->position.x, (int)ghost->position.y };

	if (ghost->update_path_timer <= 0 || SDL_Point_Equals(&dst, &org)) {
		ghost->update_path_timer = PATH_UPDATE_FREQ;
		update_path(ghost, player_pos, map);
	}

	if (dst.x > org.x)
		ghost->position.x += GHOST_SPEED * delta_time / 1000;
	else if (dst.x < org.x)
		ghost->position.x -= GHOST_SPEED * delta_time / 1000;

	if (dst.y > org.y)
		ghost->position.y += GHOST_SPEED * delta_time / 1000;
	else if (dst.y < org.y)
		ghost->position.y -= GHOST_SPEED * delta_time / 1000;
}

void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset) {
	SDL_Rect dst = { camera_offset->x + ghost->position.x * 16, camera_offset->y + ghost->position.y * 16, 16, 16 };
	SDL_RenderCopy(renderer, ghost->texture, &ghost->src, &dst);

	for (int i = 0; i < ghost->path_length; i++) {
		Uint8 r = 255 - ((ghost->path_length - i) * 255);
		SDL_SetRenderDrawColor(renderer, r, 255, 255, 255);
		SDL_Rect dst = { ghost->path[i].x * 16 + camera_offset->x, ghost->path[i].y * 16 + camera_offset->y, 16, 16 };
		SDL_RenderDrawRect(renderer, &dst);
	}
}