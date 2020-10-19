
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

#define POWERUP_MAX_TIME 10000
#define PAC_AMOUNT 240

#define PTS_FOR_NEW_LIFE 100000
#define STARTING_LIVES 2

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

typedef struct Scores {
	int lives;
	int score;
	int new_life_pts;
	int pac_left;
} Scores;

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

	Scores *scores;

	SDL_Point camera_position;

} Game;

// Load all resources
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
			-1, -1, -1, -1, -1, 05, -2, 05, 05, -1, 00, 02, 02, 02, 02, 02, 02, 01, -1, 05, 05, -2, 05, -1, -1, -1, -1, -1,
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

	Scores *scores = calloc(1, sizeof(Scores));
	game->scores = scores;

	game->camera_position.x = 0;
	game->camera_position.y = 16;

	return game;
}

// Reset game state to starting
static void init_game(Scores *scores) {
	scores->score = 0;
	scores->pac_left = PAC_AMOUNT;
	scores->new_life_pts = PTS_FOR_NEW_LIFE;
	scores->lives = STARTING_LIVES;
	scores->new_life_pts = PTS_FOR_NEW_LIFE;
}

#pragma region UPDATE
/*************
    UPDATE
**************/

static void switch_state(Game *game, State new_state) {
	game->state.state = new_state;
	switch (new_state) {
		case STATE_START_LEVEL: {
			SDL_Log("State changed to START");
			init_game(game->scores);
			switch_state(game, STATE_WAIT);
		} break;

		case STATE_WAIT: {
			SDL_Log("State changed to WAIT");
			game->player->direction = WEST;
			game->player->pos.x = 14.0f;
			game->player->pos.y = 23.5f;
			game->state.wait_state_data.timer = 5000;
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

static Tile get_tile_at_pos(const int x, const int y, const Map *map) {
	if (x < 0 || x >= map->rect.w || y < 0 || y >= map->rect.h)
		return EMPTY;

	return map->map[x + y * map->rect.w];
}

static void toggle_map_color(SDL_Texture *texture) {
	Uint8 r = 0;
	SDL_GetTextureColorMod(texture, &r, NULL, NULL);

	if (r == 255)
		SDL_SetTextureColorMod(texture, 0, 0, 255);
	else
		SDL_SetTextureColorMod(texture, 255, 255, 255);
}

static void eat_at_pos(const int x, const int y, Map *map, Scores *scores, NormalStateData *data) {
	switch (get_tile_at_pos(x, y, map)) {
		case PAC:
			map->map[x + y * map->rect.w] = EMPTY;
			scores->score += 100;
			scores->new_life_pts -= 100;
			scores->pac_left--;
			break;
		case POWERUP:
			map->map[x + y * map->rect.w] = EMPTY;
			data->power_up_timer = POWERUP_MAX_TIME;
			data->blink_timer = 200;
			break;
	}
}

static void take_damage(Game *game) {
	if (game->scores->lives-- <= 0) {
		switch_state(game, STATE_GAMEOVER);
		return;
	}
	switch_state(game, STATE_WAIT);
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
			case SDL_SCANCODE_K:
				take_damage(game);
				break;
		}
	}
}

static void handle_player_movement(int delta_time, Player *player, Map *map, float min_x, float max_x) {
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

	// Wrap around
	if (player->pos.x < min_x) {
		player->pos.x = max_x;
	}
	if (player->pos.x > max_x) {
		player->pos.x = min_x;
	}
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
			handle_player_movement(delta_time, game->player, game->map, game->camera_position.x, game->camera_position.x + game->map->rect.w);
			eat_at_pos((int)game->player->pos.x, (int)game->player->pos.y, game->map, game->scores, &game->state.normal_state_data);
			if (game->scores->new_life_pts <= 0) {
				game->scores->lives++;
				game->scores->new_life_pts += 10000;
			}

			if (game->scores->pac_left <= 0) {
				switch_state(game, STATE_WIN);
			}

			NormalStateData *data = &game->state.normal_state_data;
			if (data->power_up_timer > 0) {
				data->power_up_timer -= delta_time;
				data->blink_timer -= delta_time;

				if (data->power_up_timer < 0) {
					data->power_up_timer = 0;
					SDL_SetTextureColorMod(game->map->texture, 0, 0, 255);
				}

				if (data->blink_timer < 0) {
					toggle_map_color(game->map->texture);
					if (data->power_up_timer > 2000)
						data->blink_timer = 200;
					else
						data->blink_timer = 100;
				}
			}
		} break;

		default:
			break;
	}
}

#pragma endregion

#pragma region DRAWING
/**************
    DRAWING
***************/

static int draw_text(SDL_Renderer *renderer, TTF_Font *font, char *text, const SDL_Point *src) {
	SDL_Color color = { 255, 255, 255, 255 };
	SDL_Surface *surf = TTF_RenderText_Solid(font, text, color);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	SDL_FreeSurface(surf);
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	SDL_Rect dst = { src->x, src->y, w, h };

	SDL_RenderCopy(renderer, texture, NULL, &dst);
	SDL_DestroyTexture(texture);

	return w + src->x;
}

static void draw_ui(SDL_Renderer *renderer, TTF_Font *font, Scores *scores, SDL_Texture *player_texture, State current_state) {
	SDL_Point place = { 0, 0 };

	// Score
	char score_str[16];
	sprintf_s(score_str, 16 * sizeof(char), "Score : %06d", scores->score);
	place.x = draw_text(renderer, font, score_str, &place);

	place.x += 16;

	char new_life_str[16];
	sprintf_s(new_life_str, 16 * sizeof(char), "1UP : %06d", scores->new_life_pts);
	place.x = draw_text(renderer, font, new_life_str, &place);

	place.x += 16;

	// Lives
	for (int i = 0; i < scores->lives; i++) {
		SDL_Rect src = { 0, 0, 16, 16 };
		SDL_Rect dst = { place.x, 0, 16, 16 };
		place.x += 16;
		SDL_RenderCopy(renderer, player_texture, &src, &dst);
	}

	switch (current_state) {
		case STATE_WAIT: {
			place.x = draw_text(renderer, font, "WAIT", &place);
		} break;
		case STATE_NORMAL: {
			place.x = draw_text(renderer, font, "NORMAL", &place);
		} break;
		case STATE_WIN: {
			place.x = draw_text(renderer, font, "WIN", &place);
		} break;
	}
}

static void draw_player(SDL_Renderer *renderer, Player *player, SDL_Point *camera_offset) {
	SDL_Rect src = { player->direction % 2 * 16, player->direction / 2 * 16, 16, 16 };
	SDL_Rect dst = { (int)(player->pos.x * 16.0f) - 8 + camera_offset->x, (int)(player->pos.y * 16.0f) - 8 + camera_offset->y, 16, 16 };

	SDL_RenderCopy(renderer, player->texture, &src, &dst);
}

static void draw_map(SDL_Renderer *renderer, Map *map, SDL_Point *camera_offset) {
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
					SDL_Rect pac = { x * 16 + 6 + camera_offset->x, y * 16 + 6 + camera_offset->y, 4, 4 };
					SDL_RenderDrawRect(renderer, &pac);
				} break;
				case POWERUP: {
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
					SDL_Rect pup = { x * 16 + 2 + camera_offset->x, y * 16 + 2 + camera_offset->y, 14, 14 };
					SDL_RenderDrawRect(renderer, &pup);
				} break;
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

	draw_ui(renderer, game->font, game->scores, game->player->texture, game->state.state);

	SDL_RenderPresent(renderer);
}

#pragma endregion

static void destroy_game(Game *game) {
	TTF_CloseFont(game->font);

	SDL_DestroyTexture(game->player->texture);
	free(game->player);

	SDL_DestroyTexture(game->map->texture);
	free(game->map);

	free(game->scores);

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
