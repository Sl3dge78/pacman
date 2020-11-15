#include "game.h"

/*
 * DECLARATIONS
 */

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

	STATE_NEW_GAME,
	STATE_START_LEVEL,
	STATE_WAIT,
	STATE_DEATH,
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

	int level;
	int lives;
	int score;
	int new_life_pts;
	int pac_left;

	bool is_powered_up;

	Mix_Music *intro_bgm;
	Mix_Chunk *death_sfx;
	Mix_Chunk *waka_sfx;

} Game;

static void switch_state(Game *game, State new_state);
static void init_level(Game *game);

/*
 *  UPDATE
 */

static void switch_state(Game *game, State new_state) {
	game->state.state = new_state;
	switch (new_state) {
		case STATE_NEW_GAME: {
			game->score = 0;
			game->new_life_pts = PTS_FOR_NEW_LIFE;
			game->lives = STARTING_LIVES;
			game->new_life_pts = PTS_FOR_NEW_LIFE;
			game->pac_left = PAC_AMOUNT;
			switch_state(game, STATE_START_LEVEL);
		} break;

		case STATE_START_LEVEL: {
			SDL_Log("State changed to START");
			init_level(game);
			Mix_PlayMusic(game->intro_bgm, 1);
			switch_state(game, STATE_WAIT);
		} break;

		case STATE_WAIT: {
			SDL_Log("State changed to WAIT");
			game->state.wait_state_data.timer = 5500;

			player_reset(game->player);

			float ghost_speed = 2.0f + game->level * 0.5f;
			for (int i = 0; i < GHOST_AMT; i++) {
				ghost_reset(game->ghosts[i], ghost_speed);
			}
			map_reset_color(game->map);
		} break;

		case STATE_NORMAL: {
			SDL_Log("State changed to NORMAL");
			game->state.normal_state_data.blink_timer = 0;
			game->state.normal_state_data.power_up_timer = 0;
		} break;

		case STATE_DEATH: {
			game->state.kill_state_data.kill_timer = 2000;
			player_kill(game->player);
		} break;

		case STATE_WIN: {
			SDL_Log("WIN!");
			game->level++;
			switch_state(game, STATE_START_LEVEL);
		} break;

		case STATE_GAMEOVER: {
			SDL_Log("GAMEOVER!");
			//game->is_running = false;
		}
	}
}

static void input(SDL_Event *e, Game *game, Player *player) {
	switch (game->state.state) {
		case STATE_NORMAL: {
			player_input(game->player, e);
		} break;
		case STATE_GAMEOVER: {
			if (e->type == SDL_KEYDOWN) {
				switch_state(game, STATE_NEW_GAME);
			}
		} break;
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
				update_ghost(game->ghosts[i], delta_time, player_get_pos(game->player), game->map);
			}

			player_update(game->player, delta_time, game->map, game->camera_position.x, game->camera_position.x + MAP_WIDTH);

			switch (map_eat_at(game->map, player_get_pos(game->player)->x + 0.5f, player_get_pos(game->player)->y + 0.5f)) {
				case PAC:
					Mix_PlayChannel(0, game->waka_sfx, 0);
					game->score += 100;
					game->new_life_pts -= 100;
					game->pac_left--;
					break;
				case POWERUP:
					game->is_powered_up = true;
					game->state.normal_state_data.power_up_timer = POWERUP_MAX_TIME;
					game->state.normal_state_data.blink_timer = 200;
					for (int i = 0; i < GHOST_AMT; i++) {
						ghost_switch_state(game->ghosts[i], FLEEING);
					}
					break;
			}
			if (game->pac_left <= 0) {
				switch_state(game, STATE_WIN);
			}
			if (game->new_life_pts <= 0) {
				game->lives++;
				game->new_life_pts += 100000;
			}
			if (game->is_powered_up) {
				NormalStateData *data = &game->state.normal_state_data;
				if (data->power_up_timer >= 0) {
					data->power_up_timer -= delta_time;
					data->blink_timer -= delta_time;
				}

				if (data->blink_timer < 0) {
					map_toggle_color(game->map);
					if (data->power_up_timer > 2000)
						data->blink_timer = 200;
					else
						data->blink_timer = 100;
				}

				if (data->power_up_timer < 0) {
					data->power_up_timer = 0;
					game->is_powered_up = false;

					map_reset_color(game->map);
					for (int i = 0; i < GHOST_AMT; i++) {
						ghost_switch_state(game->ghosts[i], ATTACKING);
					}
				}
			}
			for (int i = 0; i < GHOST_AMT; i++) {
				if (intersect_sprites(player_get_pos(game->player), ghost_get_pos(game->ghosts[i]))) {
					if (game->is_powered_up) {
						ghost_kill(game->ghosts[i]);
						game->score += 1000;
					} else {
						switch_state(game, DEAD);
					}
				}
			}
		} break;

		case STATE_DEATH: {
			Mix_PlayChannel(-1, game->death_sfx, 0);
			player_play_death_animation(game->player, delta_time);
			KillStateData *data = &game->state.kill_state_data;
			if (data->kill_timer > 0) {
				data->kill_timer -= delta_time;
			} else if (game->lives-- <= 0) {
				switch_state(game, STATE_GAMEOVER);
			} else {
				switch_state(game, STATE_WAIT);
			}
		}
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

static void draw_ui(SDL_Renderer *renderer, SDL_Window *window, Game *game) {
	SDL_Point place = { 0, 0 };

	// Score
	char score_str[16];
	sprintf_s(score_str, 16 * sizeof(char), "Score : %06d", game->score);
	place.x = draw_text(renderer, game->font, score_str, &place, ALIGN_LEFT);

	place.x += 16;

	char new_life_str[16];
	sprintf_s(new_life_str, 16 * sizeof(char), "1UP : %06d", game->new_life_pts);
	place.x = draw_text(renderer, game->font, new_life_str, &place, ALIGN_LEFT);

	place.x += 16;

	// Lives
	for (int i = 0; i < game->lives; i++) {
		SDL_Rect src = { 0, 0, 16, 16 };
		SDL_Rect dst = { place.x, 0, 16, 16 };
		place.x += 16;
		SDL_RenderCopy(renderer, player_get_texture(game->player), &src, &dst);
	}

	place.x += 16;

	// Level
	char level_str[10];
	int w = 0;
	SDL_GetWindowSize(window, &w, NULL);
	place.x = w;
	sprintf_s(level_str, 16 * sizeof(char), "Level : %03d", game->level);
	place.x = draw_text(renderer, game->font, level_str, &place, ALIGN_RIGHT);

	if (game->state.state == STATE_WAIT) {
		SDL_Point ready_pos = { 14.5f * 16.0f, 18.5f * 16.0f };
		draw_text(renderer, game->font, "GET READY !", &ready_pos, ALIGN_CENTERED);
	}
	if (game->state.state == STATE_GAMEOVER) {
		SDL_Point game_over_pos = { 14.5f * 16.0f, 18.5f * 16.0f };
		draw_text(renderer, game->font, "GAME OVER !", &game_over_pos, ALIGN_CENTERED);
	}
}

static void draw(SDL_Renderer *renderer, SDL_Window *window, Game *game) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	map_draw(game->map, renderer, &game->camera_position);
	player_draw(game->player, renderer, &game->camera_position);

	for (int i = 0; i < GHOST_AMT; i++) {
		draw_ghost(renderer, game->ghosts[i], &game->camera_position);
		dbg_draw_ghost(game->ghosts[i], renderer, game->font, &game->camera_position);
	}

	draw_ui(renderer, window, game);

	SDL_RenderPresent(renderer);
}

/* 
 * CORE
 */

static Game *load_resources(SDL_Renderer *renderer) {
	Game *game = malloc(sizeof(Game));

	game->is_running = true;

	game->font = TTF_OpenFont("resources/unifont.ttf", 16);

	game->player = player_load(renderer);

	game->map = map_load(renderer);

	game->camera_position.x = 0;
	game->camera_position.y = 16;

	game->ghosts[0] = create_ghost(renderer, 13.5f, 11, 0, 0, 0);

	game->ghosts[1] = create_ghost(renderer, 11.5f, 14, 5000, 0, 16);
	game->ghosts[2] = create_ghost(renderer, 13.5f, 14, 10000, 16, 0);
	game->ghosts[3] = create_ghost(renderer, 15.5f, 14, 15000, 16, 16);

	game->level = 1;
	game->score = 0;
	game->new_life_pts = PTS_FOR_NEW_LIFE;
	game->lives = STARTING_LIVES;
	game->new_life_pts = PTS_FOR_NEW_LIFE;
	game->pac_left = PAC_AMOUNT;

	game->intro_bgm = Mix_LoadMUS("resources/audio/intro.wav");
	game->death_sfx = Mix_LoadWAV("resources/audio/death.wav");
	game->waka_sfx = Mix_LoadWAV("resources/audio/waka.wav");

	return game;
}

static void init_level(Game *game) {
	game->is_powered_up = false;
	game->pac_left = PAC_AMOUNT;
	reset_map(game->map);
}

static void destroy_game(Game *game) {
	for (int i = 0; i < GHOST_AMT; i++) {
		destroy_ghost(game->ghosts[i]);
	}

	player_free(game->player);

	Mix_FreeMusic(game->intro_bgm);
	Mix_FreeChunk(game->death_sfx);
	Mix_FreeChunk(game->waka_sfx);

	TTF_CloseFont(game->font);

	free(game);
}

void run(SDL_Renderer *renderer, SDL_Window *window) {
	Game *game = load_resources(renderer);

	switch_state(game, STATE_NEW_GAME);

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

				input(&e, game, game->player);
			}

			update(delta_time, game);
			draw(renderer, window, game);
		}
	}

	destroy_game(game);
}
