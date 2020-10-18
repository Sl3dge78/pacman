
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <stdio.h>

#include "Utils.h"

#define FPS 60
#define FRAME_TIME 1.0f / (float)FPS

#define MAP_WIDTH 28
#define MAP_HEIGHT 31
#define MAP_SIZE MAP_WIDTH *MAP_HEIGHT

enum {
	PAC = -2,
	POWERUP = -3,
	EMPTY = -1,
	TURN_RIGHT = 0,
	TURN_DOWN = 1,
	STRAIGHT_HOR = 2,
	TURN_UP = 3,
	TURN_LEFT = 4,
	STRAIGHT_VER = 5,
} typedef Tile;

enum {
	EAST = 0,
	SOUTH = 1,
	NORTH = 2,
	WEST = 3,
} typedef Direction;

typedef Tile TileMap[MAP_SIZE];

typedef struct Map {
	TileMap map;
	SDL_Texture *texture;
	SDL_Rect rect;

} Map;

typedef struct Player {
	SDL_FPoint pos;
	SDL_Texture *texture;
	Direction direction;

} Player;

//GAME DATA
static bool running = true;
static Player *player = NULL;
static Map *map = NULL;
static TTF_Font *font = NULL;

// GAMEPLAY
static int score = 0;
static int power_up_timer = 0;
static int blink_timer = 0;
static const int POWERUP_MAX_TIME = 10000;
static int pac_left = 240;
static const SDL_Point OFFSET = { 0, 16 };

// Load all resources
static void load_resources(SDL_Renderer *renderer) {
	font = TTF_OpenFont("resources/unifont.ttf", 16);

	player = calloc(1, sizeof(Player));
	player->direction = WEST;
	player->pos.x = 13.5f;
	player->pos.y = 23.5f;
	player->texture = IMG_LoadTexture(renderer, "resources/pac_man.png");

	map = calloc(1, sizeof(Map));
	{
		Map map2 = {
			00, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 01, 00, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 01,
			05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05,
			05, -2, 00, 02, 02, 01, -2, 00, 02, 02, 02, 01, -2, 05, 05, -2, 00, 02, 02, 02, 01, -2, 00, 02, 02, 01, -2, 05,
			05, -3, 05, -1, -1, 05, -2, 05, -1, -1, -1, 05, -2, 05, 05, -2, 05, -1, -1, -1, 05, -2, 05, -1, -1, 05, -3, 05,
			05, -2, 03, 02, 02, 04, -2, 03, 02, 02, 02, 04, -2, 03, 04, -2, 03, 02, 02, 02, 04, -2, 03, 02, 02, 04, -2, 05,
			05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05,
			05, -2, 00, 02, 02, 01, -2, 00, 01, -2, 00, 02, 02, 02, 02, 02, 02, 01, -2, 00, 01, -2, 00, 02, 02, 01, -2, 05,
			05, -2, 03, 02, 02, 04, -2, 05, 05, -2, 03, 02, 02, 01, 00, 02, 02, 04, -2, 05, 05, -2, 03, 02, 02, 04, -2, 05,
			05, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, 05,
			03, 02, 02, 02, 02, 01, -2, 05, 03, 02, 02, 01, -1, 05, 05, -1, 00, 02, 02, 04, 05, -2, 00, 02, 02, 02, 02, 04,
			-1, -1, -1, -1, -1, 05, -2, 05, 00, 02, 02, 04, -1, 03, 04, -1, 03, 02, 02, 01, 05, -2, 05, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 00, 02, 02, 02, 02, 02, 02, 01, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			02, 02, 02, 02, 02, 04, -2, 03, 04, -1, 05, -1, -1, -1, -1, -1, -1, 05, -1, 03, 04, -2, 03, 02, 02, 02, 02, 02,
			-1, -1, -1, -1, -1, -1, -2, -1, -1, -1, 05, -1, -1, -1, -1, -1, -1, 05, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1,
			02, 02, 02, 02, 02, 01, -2, 00, 01, -1, 05, -1, -1, -1, -1, -1, -1, 05, -1, 00, 01, -2, 00, 02, 02, 02, 02, 02,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 03, 02, 02, 02, 02, 02, 02, 04, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 00, 02, 02, 02, 02, 02, 02, 01, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			00, 02, 02, 02, 02, 04, -2, 03, 04, -1, 03, 02, 02, 01, 00, 02, 02, 04, -1, 03, 04, -2, 03, 02, 02, 02, 02, 01,
			05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05,
			05, -2, 00, 02, 02, 01, -2, 00, 02, 02, 02, 01, -2, 05, 05, -1, 00, 02, 02, 02, 01, -2, 00, 02, 02, 01, -2, 05,
			05, -2, 03, 02, 01, 05, -2, 03, 02, 02, 02, 04, -2, 03, 04, -1, 03, 02, 02, 02, 04, -2, 05, 00, 02, 04, -2, 05,
			05, -3, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -3, 05,
			03, 02, 01, -2, 05, 05, -2, 00, 01, -2, 00, 02, 02, 02, 02, 02, 02, 01, -2, 00, 01, -2, 05, 05, -2, 00, 02, 04,
			00, 02, 04, -2, 03, 04, -2, 05, 05, -2, 03, 02, 02, 01, 00, 02, 02, 04, -2, 05, 05, -2, 03, 04, -2, 03, 02, 01,
			05, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, 05,
			05, -2, 00, 02, 02, 02, 02, 04, 03, 02, 02, 01, -2, 05, 05, -2, 00, 02, 02, 04, 03, 02, 02, 02, 02, 01, -2, 05,
			05, -2, 03, 02, 02, 02, 02, 02, 02, 02, 02, 04, -2, 03, 04, -2, 03, 02, 02, 02, 02, 02, 02, 02, 02, 04, -2, 05,
			05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05,
			03, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 04
		};
		SDL_memcpy(map->map, &map2, MAP_SIZE * sizeof(Tile));
	}
	map->texture = IMG_LoadTexture(renderer, "resources/walls.png");
	SDL_SetTextureColorMod(map->texture, 0, 0, 255);
	map->rect.x = 16;
	map->rect.y = 16;
	map->rect.w = MAP_WIDTH;
	map->rect.h = MAP_HEIGHT;
}

// Reset game state to starting
static void init_game() {
	score = 0;
	pac_left = 240;

	player->direction = WEST;
	player->pos.x = 13.5f;
	player->pos.y = 23.5f;

	SDL_SetTextureColorMod(map->texture, 0, 0, 255);
}

/*************
    UPDATE
**************/

static Tile get_tile_at_pos(const int x, const int y, const Map *map) {
	if (x < 0 || x >= map->rect.w || y < 0 || y >= map->rect.h)
		return EMPTY;

	return map->map[x + y * map->rect.w];
}

static void on_power_up_start() {
	power_up_timer = POWERUP_MAX_TIME;
	blink_timer = 200;
}

static void on_power_up_end() {
	SDL_SetTextureColorMod(map->texture, 0, 0, 255);
}

static void toggle_map_color(SDL_Texture *texture) {
	Uint8 r = 0;
	SDL_GetTextureColorMod(texture, &r, NULL, NULL);

	if (r == 255)
		SDL_SetTextureColorMod(map->texture, 0, 0, 255);
	else
		SDL_SetTextureColorMod(map->texture, 255, 255, 255);
}

static void eat_at_pos(const int x, const int y, Map *map) {
	switch (get_tile_at_pos(x, y, map)) {
		case PAC:
			map->map[x + y * map->rect.w] = EMPTY;
			score += 100;
			pac_left--;
			break;
		case POWERUP:
			map->map[x + y * map->rect.w] = EMPTY;
			on_power_up_start();
			break;
	}
}

void input(SDL_Event *e) {
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

void update(const int delta_time) {
	{
		float speed = 5.0f * delta_time / 1000.0f;
		SDL_FPoint new_pos = player->pos;
		switch (player->direction) {
			case NORTH: {
				if (get_tile_at_pos((int)player->pos.x, (int)(player->pos.y - speed - .5f), map) < 0)
					new_pos.y -= speed;
			} break;
			case SOUTH: {
				if (get_tile_at_pos((int)player->pos.x, (int)(player->pos.y + speed + .5f), map) < 0)
					new_pos.y += speed;
			} break;
			case EAST: {
				if (get_tile_at_pos((int)(player->pos.x + speed + 0.5f), (int)player->pos.y, map) < 0)
					new_pos.x += speed;
			} break;
			case WEST: {
				if (get_tile_at_pos((int)(player->pos.x - speed - 0.5f), (int)player->pos.y, map) < 0)
					new_pos.x -= speed;
			} break;
		}
		player->pos = new_pos;

		eat_at_pos((int)player->pos.x, (int)player->pos.y, map);

		// Wrap around
		if (player->pos.x < OFFSET.x) {
			player->pos.x = map->rect.w + OFFSET.x;
		}
		if (player->pos.x > map->rect.w + OFFSET.x) {
			player->pos.x = OFFSET.x;
		}
	}

	if (power_up_timer > 0) {
		power_up_timer -= delta_time;
		blink_timer -= delta_time;

		if (power_up_timer < 0) {
			power_up_timer = 0;
			on_power_up_end();
		}

		if (blink_timer < 0) {
			toggle_map_color(map->texture);
			if (power_up_timer > 2000)
				blink_timer = 200;
			else
				blink_timer = 100;
		}
	}

	if (pac_left <= 0) {
		running = false;
	}
}

/**************
    DRAWING
***************/

static void draw_text(SDL_Renderer *renderer, char *text, const SDL_Point *src) {
	SDL_Color color = { 255, 255, 255, 255 };
	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	SDL_Rect dst = { src->x, src->y, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);
}

static void draw_ui(SDL_Renderer *renderer) {
	char score_str[16];
	sprintf_s(score_str, 16 * sizeof(char), "Score : %06d", score);
	SDL_Point place = { 0, 0 };
	draw_text(renderer, score_str, &place);
}

static void draw_player(SDL_Renderer *renderer) {
	SDL_Rect src = { player->direction % 2 * 16, player->direction / 2 * 16, 16, 16 };
	SDL_Rect dst = { (int)(player->pos.x * 16.0f) - 8 + OFFSET.x, (int)(player->pos.y * 16.0f) - 8 + OFFSET.y, 16, 16 };

	SDL_RenderCopy(renderer, player->texture, &src, &dst);
}

static void draw_map(SDL_Renderer *renderer) {
	SDL_Rect src = { 0, 0, 16, 16 };
	SDL_Rect dst = { 0, 0, 16, 16 };

	for (Uint32 y = 0; y < map->rect.h; y++) {
		for (Uint32 x = 0; x < map->rect.w; x++) {
			Uint32 val = x + y * map->rect.h;

			switch (map->map[x + y * map->rect.w]) {
				case EMPTY: {
					continue;
				} break;
				case PAC: {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_Rect pac = { x * 16 + 6 + OFFSET.x, y * 16 + 6 + OFFSET.y, 4, 4 };
					SDL_RenderDrawRect(renderer, &pac);
				} break;
				case POWERUP: {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_Rect pup = { x * 16 + 2 + OFFSET.x, y * 16 + 2 + OFFSET.y, 14, 14 };
					SDL_RenderDrawRect(renderer, &pup);
				} break;
				default: {
					src.x = map->map[x + y * map->rect.w] % 3 * 16;
					src.y = map->map[x + y * map->rect.w] / 3 * 16;
					dst.x = x * 16 + OFFSET.x;
					dst.y = y * 16 + OFFSET.y;
					SDL_RenderCopy(renderer, map->texture, &src, &dst);
				} break;
			}
		}
	}
}

void draw(SDL_Renderer *renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	draw_map(renderer);
	draw_player(renderer);
	draw_ui(renderer);

	SDL_RenderPresent(renderer);
}

void destroy_game() {
	TTF_CloseFont(font);

	SDL_DestroyTexture(player->texture);
	free(player);

	SDL_DestroyTexture(map->texture);
	free(map);
}

void run(SDL_Renderer *renderer) {
	load_resources(renderer);
	init_game();

	int last_time = 0;

	while (running) {
		int time = SDL_GetTicks();
		int delta_time = time - last_time;

		if (delta_time >= FRAME_TIME) {
			last_time = time;

			SDL_Event e;
			if (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) {
					running = false;
				}

				input(&e);
			}

			update(delta_time);
			draw(renderer);
		}
	}

	destroy_game();
}
