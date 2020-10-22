#include "game.h"

#include <math.h>
#include <stdio.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include "debug.h"
#include "utils.h"

#include "a_star.h"
#include "ghost.h"

/*
 * DECLARATIONS
 */

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

	int lives;
	int score;
	int new_life_pts;
	int pac_left;

	bool is_powered_up;

} Player;

typedef struct WaitStateData {
	int timer;
} WaitStateData;

typedef struct NormalStateData {
	int power_up_timer;
	int blink_timer;

} NormalStateData;

typedef struct KillStateData {
	int kill_timer;
} KillStateData;

enum State {

	STATE_START_LEVEL,
	STATE_WAIT,
	STATE_NORMAL,
	STATE_WIN,
	STATE_GAMEOVER

} typedef State;

typedef struct GameState {
	State state;
	union {
		WaitStateData wait_state_data;
		NormalStateData normal_state_data;
		KillStateData kill_state_data;
	};
} GameState;

typedef struct Game {
	GameState state;

	Player *player;
	Map *map;
	TTF_Font *font;

	bool is_running;

	SDL_Point camera_position;
	Ghost *ghosts[4];

} Game;

static void switch_state(Game *game, State new_state);
static void init_game(Player *player);

/*
 * MAP
 */

static int get_tile_id(const int x, const int y, const Map *map) {
	return x + y * map->rect.w;
}

Tile get_tile_at_pos(const int x, const int y, const Map *map) {
	if (x < 0 || x >= map->rect.w || y < 0 || y >= map->rect.h)
		return OUTMAP;

	return map->map[get_tile_id(x, y, map)];
}

static void toggle_map_color(SDL_Texture *texture) {
	Uint8 r = 0;
	SDL_GetTextureColorMod(texture, &r, NULL, NULL);

	if (r == 255)
		SDL_SetTextureColorMod(texture, 0, 0, 255);
	else
		SDL_SetTextureColorMod(texture, 255, 255, 255);
}

/*
 * PLAYER
 */
static void take_damage(Game *game) {
	if (game->player->lives-- <= 0) {
		switch_state(game, STATE_GAMEOVER);
		return;
	}
	switch_state(game, STATE_WAIT);
}

static void handle_player_movement(int delta_time, Player *player, Map *map, float min_x, float max_x) {
	float speed = 5.0f * delta_time / 1000.0f;
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
	switch (get_tile_at_pos((int)bound_pos.x, (int)bound_pos.y, map)) {
		case EMPTY:
		case OUTMAP:
		case PAC:
		case POWERUP:
			player->pos = new_pos;
			break;

		default:
			break;
	}

	// Wrap around
	if (player->pos.x < min_x) {
		player->pos.x = max_x;
	}
	if (player->pos.x > max_x) {
		player->pos.x = min_x;
	}
}

/*
 *  UPDATE
 */

static void switch_state(Game *game, State new_state) {
	game->state.state = new_state;
	switch (new_state) {
		case STATE_START_LEVEL: {
			SDL_Log("State changed to START");
			init_game(game->player);
			switch_state(game, STATE_WAIT);
		} break;

		case STATE_WAIT: {
			SDL_Log("State changed to WAIT");
			game->player->direction = WEST;
			game->player->pos.x = 13.0f;
			game->player->pos.y = 23.0f;
			game->state.wait_state_data.timer = 5000;
			for (int i = 0; i < GHOST_AMT; i++) {
				ghost_reset(game->ghosts[i]);
			}
			SDL_SetTextureColorMod(game->map->texture, 0, 0, 255);
		} break;

		case STATE_NORMAL: {
			SDL_Log("State changed to NORMAL");
			game->state.normal_state_data.blink_timer = 0;
			game->state.normal_state_data.power_up_timer = 0;
		} break;

		case STATE_WIN: {
			SDL_Log("WIN!");
			game->is_running = false;
		} break;

		case STATE_GAMEOVER: {
			SDL_Log("GAMEOVER!");
			game->is_running = false;
		}
	}
}

static void input(SDL_Event *e, Game *game, Player *player) {
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
			case SDL_SCANCODE_1: {
			} break;
		}
	}
}

static bool intersect_sprites(const SDL_FPoint *a, const SDL_FPoint *b) {
	if (a->x + 1.0f >= b->x && a->x + 1.0f <= b->x + 1.0f && a->y + 1.0f >= b->y && a->y + 1.0f <= b->y + 1.0f)
		return true;
	if (a->x + 1.0f <= b->x && a->x + 1.0f >= b->x + 1.0f && a->y >= b->y && a->y <= b->y + 1.0f)
		return true;
	if (a->x >= b->x && a->x <= b->x + 1.0f && a->y >= b->y && a->y <= b->y + 1.0f)
		return true;
	if (a->x >= b->x && a->x <= b->x + 1.0f && a->y + 1.0f >= b->y && a->y + 1.0f <= b->y + 1.0f)
		return true;

	return false;
}

static void update(const int delta_time, Game *game) {
	switch (game->state.state) {
		case STATE_WAIT: {
			WaitStateData *data = &game->state.wait_state_data;

			if (data->timer > 0) {
				data->timer -= delta_time;
			} else {
				switch_state(game, STATE_NORMAL);
			}

		} break;
		case STATE_NORMAL: {
			for (int i = 0; i < GHOST_AMT; i++) {
				update_ghost(game->ghosts[i], delta_time, &game->player->pos, game->map);
			}

			Player *player = game->player;
			handle_player_movement(delta_time, player, game->map, game->camera_position.x, game->camera_position.x + game->map->rect.w);
			int id = get_tile_id(player->pos.x + .5f, player->pos.y + .5f, game->map);
			switch (get_tile_at_pos(player->pos.x + 0.5f, player->pos.y + 0.5f, game->map)) {
				case PAC:
					game->map->map[id] = EMPTY;
					player->score += 100;
					player->new_life_pts -= 100;
					player->pac_left--;
					break;
				case POWERUP:
					player->is_powered_up = true;
					game->map->map[id] = EMPTY;
					game->state.normal_state_data.power_up_timer = POWERUP_MAX_TIME;
					game->state.normal_state_data.blink_timer = 200;
					break;
			}
			if (player->pac_left <= 0) {
				switch_state(game, STATE_WIN);
			}
			if (player->new_life_pts <= 0) {
				player->lives++;
				player->new_life_pts += 100000;
			}
			if (player->is_powered_up) {
				NormalStateData *data = &game->state.normal_state_data;
				if (data->power_up_timer > 0) {
					data->power_up_timer -= delta_time;
					data->blink_timer -= delta_time;

					if (data->blink_timer < 0) {
						toggle_map_color(game->map->texture);
						if (data->power_up_timer > 2000)
							data->blink_timer = 200;
						else
							data->blink_timer = 100;
					}

					if (data->power_up_timer < 0) {
						data->power_up_timer = 0;
						SDL_SetTextureColorMod(game->map->texture, 0, 0, 255);
						player->is_powered_up = false;
					}
				}
			}
			for (int i = 0; i < GHOST_AMT; i++) {
				if (intersect_sprites(&player->pos, ghost_get_pos(game->ghosts[i]))) {
					take_damage(game);
				}
			}

		} break;

		default:
			break;
	}
}

/*
 *  DRAWING
 */

int draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, const SDL_Point *src, Alignement align) {
	SDL_Color color = { 255, 255, 255, 255 };
	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	SDL_Rect dst = { src->x, src->y, w, h };

	switch (align) {
		case ALIGN_CENTERED:
			dst.x -= w / 2;
			dst.y -= h / 2;
			break;
		case ALIGN_LEFT:
			break;
		case ALIGN_RIGHT:
			dst.x -= w;
			break;
	}

	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);

	return w + src->x;
}

static void draw_ui(SDL_Renderer *renderer, TTF_Font *font, Player *player, State current_state) {
	SDL_Point place = { 0, 0 };

	// Score
	char score_str[16];
	sprintf_s(score_str, 16 * sizeof(char), "Score : %06d", player->score);
	place.x = draw_text(renderer, font, score_str, &place, ALIGN_LEFT);

	place.x += 16;

	char new_life_str[16];
	sprintf_s(new_life_str, 16 * sizeof(char), "1UP : %06d", player->new_life_pts);
	place.x = draw_text(renderer, font, new_life_str, &place, ALIGN_LEFT);

	place.x += 16;

	// Lives
	for (int i = 0; i < player->lives; i++) {
		SDL_Rect src = { 0, 0, 16, 16 };
		SDL_Rect dst = { place.x, 0, 16, 16 };
		place.x += 16;
		SDL_RenderCopy(renderer, player->texture, &src, &dst);
	}

	switch (current_state) {
		case STATE_WAIT: {
			SDL_Point center = { MAP_WIDTH * 16 / 2, MAP_HEIGHT * 16 / 2 + 48 };
			place.x = draw_text(renderer, font, "GET READY!", &center, ALIGN_CENTERED);
		} break;
	}
}

static void draw_player(SDL_Renderer *renderer, Player *player, SDL_Point *camera_offset) {
	SDL_Rect src = { player->direction % 2 * 16, player->direction / 2 * 16, 16, 16 };
	SDL_Rect dst = { (int)(player->pos.x * 16.0f) + camera_offset->x, (int)(player->pos.y * 16.0f) + camera_offset->y, 16, 16 };

	SDL_RenderCopy(renderer, player->texture, &src, &dst);
}

static void draw_map(SDL_Renderer *renderer, Map *map, SDL_Point *camera_offset) {
	SDL_Rect src = { 0, 0, 16, 16 };
	SDL_Rect dst = { 0, 0, 16, 16 };

	for (Uint32 y = 0; y < map->rect.h; y++) {
		for (Uint32 x = 0; x < map->rect.w; x++) {
			Uint32 val = x + y * map->rect.h;

			switch (map->map[x + y * map->rect.w]) {
				case OUTMAP:
				case EMPTY:
					continue;
					break;
				case PAC: {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_Rect pac = { x * 16 + 6 + camera_offset->x, y * 16 + 6 + camera_offset->y, 4, 4 };
					SDL_RenderFillRect(renderer, &pac);
				} break;
				case POWERUP: {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_Rect pup = { x * 16 + 2 + camera_offset->x, y * 16 + 2 + camera_offset->y, 14, 14 };
					SDL_RenderFillRect(renderer, &pup);
				} break;
				case DOOR: {
					src.x = 3 * 16;
					src.y = 0;
					dst.x = x * 16 + camera_offset->x;
					dst.y = y * 16 + camera_offset->y;
					SDL_RenderCopy(renderer, map->texture, &src, &dst);
				}
				default: {
					src.x = map->map[x + y * map->rect.w] % 3 * 16;
					src.y = map->map[x + y * map->rect.w] / 3 * 16;
					dst.x = x * 16 + camera_offset->x;
					dst.y = y * 16 + camera_offset->y;
					SDL_RenderCopy(renderer, map->texture, &src, &dst);
				} break;
			}
		}
	}
}

static void draw(SDL_Renderer *renderer, Game *game) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	draw_map(renderer, game->map, &game->camera_position);
	draw_player(renderer, game->player, &game->camera_position);

	draw_ui(renderer, game->font, game->player, game->state.state);

	for (int i = 0; i < GHOST_AMT; i++) {
		draw_ghost(renderer, game->ghosts[i], &game->camera_position);
		dbg_draw_ghost(game->ghosts[i], renderer, game->font, &game->camera_position);
	}

	SDL_RenderPresent(renderer);
}

/* 
 * CORE
 */

static Game *load_resources(SDL_Renderer *renderer) {
	Game *game = malloc(sizeof(Game));

	game->is_running = true;

	game->font = TTF_OpenFont("resources/unifont.ttf", 16);

	game->player = calloc(1, sizeof(Player));
	game->player->texture = IMG_LoadTexture(renderer, "resources/pac_man.png");

	game->map = calloc(1, sizeof(Map));
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
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 00, 02, 02, -4, -4, 02, 02, 01, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			02, 02, 02, 02, 02, 04, -2, 03, 04, -1, 05, -1, -1, -1, -1, -1, -1, 05, -1, 03, 04, -2, 03, 02, 02, 02, 02, 02,
			-1, -1, -1, -1, -1, -1, -2, -1, -1, -1, 05, -1, -1, -1, -1, -1, -1, 05, -1, -1, -1, -2, -1, -1, -1, -1, -1, -1,
			02, 02, 02, 02, 02, 01, -2, 00, 01, -1, 05, -1, -1, -1, -1, -1, -1, 05, -1, 00, 01, -2, 00, 02, 02, 02, 02, 02,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 03, 02, 02, 02, 02, 02, 02, 04, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 00, 02, 02, 02, 02, 02, 02, 01, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
			00, 02, 02, 02, 02, 04, -2, 03, 04, -1, 03, 02, 02, 01, 00, 02, 02, 04, -1, 03, 04, -2, 03, 02, 02, 02, 02, 01,
			05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05,
			05, -2, 00, 02, 02, 01, -2, 00, 02, 02, 02, 01, -2, 05, 05, -2, 00, 02, 02, 02, 01, -2, 00, 02, 02, 01, -2, 05,
			05, -2, 03, 02, 01, 05, -2, 03, 02, 02, 02, 04, -2, 03, 04, -2, 03, 02, 02, 02, 04, -2, 05, 00, 02, 04, -2, 05,
			05, -3, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -3, 05,
			03, 02, 01, -2, 05, 05, -2, 00, 01, -2, 00, 02, 02, 02, 02, 02, 02, 01, -2, 00, 01, -2, 05, 05, -2, 00, 02, 04,
			00, 02, 04, -2, 03, 04, -2, 05, 05, -2, 03, 02, 02, 01, 00, 02, 02, 04, -2, 05, 05, -2, 03, 04, -2, 03, 02, 01,
			05, -2, -2, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, 05, 05, -2, -2, -2, -2, -2, -2, 05,
			05, -2, 00, 02, 02, 02, 02, 04, 03, 02, 02, 01, -2, 05, 05, -2, 00, 02, 02, 04, 03, 02, 02, 02, 02, 01, -2, 05,
			05, -2, 03, 02, 02, 02, 02, 02, 02, 02, 02, 04, -2, 03, 04, -2, 03, 02, 02, 02, 02, 02, 02, 02, 02, 04, -2, 05,
			05, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 05,
			03, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 02, 04
		};
		SDL_memcpy(game->map->map, &map2, MAP_SIZE * sizeof(Tile));
	}
	game->map->texture = IMG_LoadTexture(renderer, "resources/walls.png");
	SDL_SetTextureColorMod(game->map->texture, 0, 0, 255);
	game->map->rect.x = 16;
	game->map->rect.y = 16;
	game->map->rect.w = MAP_WIDTH;
	game->map->rect.h = MAP_HEIGHT;

	game->camera_position.x = 0;
	game->camera_position.y = 16;

	game->ghosts[0] = create_ghost(renderer, 13, 11, 0);
	game->ghosts[1] = create_ghost(renderer, 11.5f, 14.0f, 5000);
	game->ghosts[2] = create_ghost(renderer, 13.5f, 14.0f, 10000);
	game->ghosts[3] = create_ghost(renderer, 15.5f, 14.0f, 15000);

	return game;
}

static void init_game(Player *player) {
	player->score = 0;
	player->pac_left = PAC_AMOUNT;
	player->new_life_pts = PTS_FOR_NEW_LIFE;
	player->lives = STARTING_LIVES;
	player->new_life_pts = PTS_FOR_NEW_LIFE;
	player->is_powered_up = false;
}

static void destroy_game(Game *game) {
	for (int i = 0; i < GHOST_AMT; i++) {
		destroy_ghost(game->ghosts[i]);
	}

	TTF_CloseFont(game->font);

	SDL_DestroyTexture(game->player->texture);
	free(game->player);

	SDL_DestroyTexture(game->map->texture);
	free(game->map);

	free(game);
}

void run(SDL_Renderer *renderer) {
	Game *game = load_resources(renderer);

	switch_state(game, STATE_START_LEVEL);

	int last_time = 0;

	while (game->is_running) {
		int time = SDL_GetTicks();
		int delta_time = time - last_time;

		if (delta_time >= FRAME_TIME) {
			last_time = time;

			SDL_Event e;
			if (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) {
					game->is_running = false;
				}
				if (game->state.state == STATE_NORMAL) {
					input(&e, game, game->player);
				}
			}

			update(delta_time, game);
			draw(renderer, game);
		}
	}

	destroy_game(game);
}
