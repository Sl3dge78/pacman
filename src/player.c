#include "player.h"

typedef struct Player {
	SDL_FPoint pos;
	SDL_Texture *texture;
	Direction direction;

	int animation_timer;
	int current_frame;
	bool is_dead;

} Player;

Player *player_load(SDL_Renderer *renderer) {
	Player *player = malloc(sizeof(Player));
	player->texture = IMG_LoadTexture(renderer, "resources/pac_man.png");
	player->animation_timer = 0;
	player->current_frame = 0;
	player->is_dead = false;
}

void player_free(Player *player) {
	SDL_DestroyTexture(player->texture);
	free(player);
}

void player_reset(Player *player) {
	player->direction = WEST;
	player->pos.x = 13.0f;
	player->pos.y = 23.0f;
	player->is_dead = false;
	player->current_frame = 0;
	player->animation_timer = 0;
}

void player_input(Player *player, SDL_Event *e) {
	if (e->type == SDL_KEYDOWN) {
		switch (e->key.keysym.scancode) {
			case SDL_SCANCODE_W:
				player->direction = NORTH;
				break;
			case SDL_SCANCODE_S:
				player->direction = SOUTH;
				break;
			case SDL_SCANCODE_A:
				player->direction = WEST;
				break;
			case SDL_SCANCODE_D:
				player->direction = EAST;
				break;
		}
	}
}

void player_update(Player *player, int delta_time, Map *map, float min_x, float max_x) {
	float speed = PLAYER_SPEED * delta_time / 1000.0f;
	SDL_FPoint new_pos = player->pos;
	SDL_FPoint bound_pos = player->pos;
	bound_pos.x += .5f;
	bound_pos.y += .5f; // .5 is the players' bound box

	switch (player->direction) {
		case NORTH:
			new_pos.y += -speed;
			bound_pos.y += -speed - .5f;
			new_pos.x = round(new_pos.x);
			break;
		case SOUTH:
			new_pos.y += speed;
			bound_pos.y += speed + .5f;
			new_pos.x = round(new_pos.x);
			break;
		case EAST:
			new_pos.x += speed;
			bound_pos.x += speed + .5f;
			new_pos.y = round(new_pos.y);
			break;
		case WEST:
			new_pos.x += -speed;
			bound_pos.x += -speed - .5f;
			new_pos.y = round(new_pos.y);
			break;
	}

	if (!map_get_collision(map, (int)bound_pos.x, (int)bound_pos.y, COLLISION_PLAYER)) {
		player->pos = new_pos;
		player->animation_timer += delta_time;
		if (player->animation_timer >= ANIMATION_SPEED) {
			player->animation_timer = 0;
			player->current_frame++;
			player->current_frame %= 4;
		}
	}

	// Wrap around
	if (player->pos.x < min_x) {
		player->pos.x = max_x;
	}
	if (player->pos.x > max_x) {
		player->pos.x = min_x;
	}
}

void player_draw(Player *player, SDL_Renderer *renderer, SDL_Point *camera_offset) {
	SDL_Rect src = { player->current_frame * 16, 0, 16, 16 };
	SDL_Rect dst = { (int)(player->pos.x * 16.0f) + camera_offset->x, (int)(player->pos.y * 16.0f) + camera_offset->y, 16, 16 };
	float angle = player->direction * 90;

	if (player->is_dead) {
		src.y = 16;
		angle = 0;
	}
	SDL_RenderCopyEx(renderer, player->texture, &src, &dst, angle, NULL, 0);
}

void player_kill(Player *player) {
	player->is_dead = true;
	player->current_frame = 0;
}

void player_play_death_animation(Player *player, int delta_time) {
	// Animation
	if (player->current_frame < 4) {
		player->animation_timer += delta_time;
		if (player->animation_timer >= DEATH_ANIMATION_SPEED) {
			player->animation_timer = 0;
			player->current_frame++;
		}
	}
}

const SDL_FPoint *player_get_pos(Player *player) {
	return &player->pos;
}

const SDL_FRect player_get_box(Player *player) {
	SDL_FRect rect = { player->pos.x - .5f, player->pos.y - .5f, 1.0f, 1.0f };
	return rect;
}

SDL_Texture *player_get_texture(Player *player) {
	return player->texture;
}
