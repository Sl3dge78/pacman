#include "ghost.h"

#include <stdio.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

#include "debug.h"

enum GhostState {
	WAITING,
	ATTACKING,
	FLEEING
} typedef GhostState;

struct Ghost {
	SDL_Texture *texture;
	SDL_FPoint position;
	SDL_FPoint destination;

	SDL_Rect src;

	GhostState state;
};

Ghost *create_ghost(SDL_Renderer *renderer) {
	Ghost *ghost = malloc(sizeof(Ghost));

	SDL_Surface *surf = IMG_Load("resources/ghost.png");
	ghost->texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);

	ghost->position.x = 11;
	ghost->position.y = 14;
	ghost->destination.x = 0;
	ghost->destination.y = 0;

	ghost->src.x = 0;
	ghost->src.y = 0;
	ghost->src.w = 16;
	ghost->src.h = 16;

	ghost->state = WAITING;
}

void destroy_ghost(Ghost *ghost) {
	SDL_DestroyTexture(ghost->texture);
	free(ghost);
}

void update_ghost(int delta_time, Ghost *ghost) {
	SDL_FPoint distance = {
		ghost->destination.x - ghost->position.x,
		ghost->destination.y - ghost->position.y
	};

	if (distance.x > .1f) {
		ghost->position.x += GHOST_SPEED * delta_time / 1000;
	} else if (distance.x < .1f) {
		ghost->position.x -= GHOST_SPEED * delta_time / 1000;
	}

	if (distance.y > .1f) {
		ghost->position.y += GHOST_SPEED * delta_time / 1000;
	} else if (distance.y < .1f) {
		ghost->position.y -= GHOST_SPEED * delta_time / 1000;
	}
}

void draw_ghost(SDL_Renderer *renderer, const Ghost *ghost, const SDL_Point *camera_offset) {
	SDL_Rect dst = { camera_offset->x + ghost->position.x * 16, camera_offset->y + ghost->position.y * 16, 16, 16 };
	SDL_RenderCopy(renderer, ghost->texture, &ghost->src, &dst);
}