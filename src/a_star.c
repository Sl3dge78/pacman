#include "a_star.h"

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "utils.h"

struct Node {
	SDL_Point pos;
	int f;
	int g;
	int h;
	struct Node *parent;
} typedef Node;

struct Vector {
	int length;
	Node **list;
} typedef Vector;

Vector *create_vector() {
	return calloc(1, sizeof(Vector));
}

void vector_push_back(Vector *vector, Node *node) {
	vector->length++;
	if (vector->length == 1) {
		vector->list = malloc(sizeof(Node *));
	} else {
		vector->list = realloc(vector->list, sizeof(Node *) * (vector->length));
	}

	vector->list[vector->length - 1] = node;
}

void vector_remove(Vector *vector, const int index) {
	for (int i = index; i < vector->length - 1; i++) {
		vector->list[i] = vector->list[i + 1];
	}

	vector->length--;
	if (vector->length == 0)
		free(vector->list);
	else
		vector->list = realloc(vector->list, sizeof(Node *) * (vector->length));
}

void vector_free(Vector *vector) {
	for (int i = 0; i < vector->length; i++) {
		free(vector->list[i]);
	}
	if (vector->length > 0)
		free(vector->list);
	free(vector);
}

bool vector_has(const Vector *vec, Node *node, int *index) {
	for (int i = 0; i < vec->length; i++) {
		if (vec->list[i] == node)
			SDL_Log("Pointer is equal you can optimize your code dude!");

		if (vec->list[i]->pos.x == node->pos.x && vec->list[i]->pos.y == node->pos.y) {
			if (index != NULL)
				*index = i;
			return true;
		}
	}
	return false;
}

void a_star(const Map *map, const SDL_Point *start, const SDL_Point *end, SDL_Point **path, int *length) {
	Vector *open_list = create_vector();
	Vector *closed_list = create_vector();

	Node *start_node = calloc(1, sizeof(Node));
	start_node->pos = *start;

	vector_push_back(open_list, start_node);

	while (open_list->length > 0) {
		Node *current_node = open_list->list[0];
		int index_to_delete = 0;
		for (int i = 0; i < open_list->length; i++) {
			if (current_node->f > open_list->list[i]->f) {
				current_node = open_list->list[i];
				index_to_delete = i;
			}
		}

		vector_push_back(closed_list, current_node);
		vector_remove(open_list, index_to_delete);

		if (current_node->pos.x == end->x && current_node->pos.y == end->y) {
			*length = current_node->g + 1;
			*path = realloc(*path, *length * sizeof(SDL_Point));
			int x = *length;
			for (Node *node = current_node; node != NULL; node = node->parent) {
				x--;
				(*path)[x] = node->pos;
			}
			break;
		}

		// Add adjacent nodes
		Node **children = calloc(4, sizeof(Node *));
		for (int i = 0; i < 4; i++) {
			children[i] = malloc(sizeof(Node));
			children[i]->pos = current_node->pos;
			children[i]->parent = current_node;
		}

		children[0]->pos.x--;
		children[1]->pos.x++;
		children[2]->pos.y--;
		children[3]->pos.y++;

		// For each adjacent node
		for (int i = 0; i < 4; i++) {
			if (vector_has(closed_list, children[i], NULL)) {
				free(children[i]);
				continue;
			}
			if (map_get_collision(map, children[i]->pos.x, children[i]->pos.y, COLLISION_GHOST)) {
				free(children[i]);
				continue;
			}

			children[i]->g = current_node->g + 1;
			children[i]->h = SDL_Point_Distance(&children[i]->pos, end);
			children[i]->f = children[i]->g + children[i]->h;
			int index = 0;
			if (vector_has(open_list, children[i], &index)) {
				if (children[i]->g > open_list->list[index]->g) {
					free(children[i]);
					continue;
				}
			}
			vector_push_back(open_list, children[i]);
		}

		free(children);
	}
	vector_free(open_list);
	vector_free(closed_list);
}

void reverse_a_star(const Map *map, const SDL_Point *start, const SDL_Point *place_to_flee, const int max_distance, SDL_Point **path, int *length) {
	Vector *open_list = create_vector();
	Vector *closed_list = create_vector();

	Node *start_node = calloc(1, sizeof(Node));
	start_node->pos = *start;

	int starting_distance = SDL_Point_Distance(start, place_to_flee);

	vector_push_back(open_list, start_node);
	int current_iteration = 0;

	while (open_list->length > 0) {
		Node *current_node = open_list->list[0];
		int index_to_delete = 0;
		// Pick node to work on
		for (int i = 0; i < open_list->length; i++) {
			if (current_node->f > open_list->list[i]->f) {
				current_node = open_list->list[i];
				index_to_delete = i;
			}
		}

		vector_push_back(closed_list, current_node);
		vector_remove(open_list, index_to_delete);

		if (current_node->h <= -max_distance - starting_distance) { // Exit point
			*length = current_node->g + 1;
			*path = realloc(*path, *length * sizeof(SDL_Point));
			int x = *length;
			for (Node *node = current_node; node != NULL; node = node->parent) {
				x--;
				(*path)[x] = node->pos;
			}
			break; // while
		}
		current_iteration++;

		// Add adjacent nodes
		Node **children = calloc(4, sizeof(Node *));
		for (int i = 0; i < 4; i++) {
			children[i] = malloc(sizeof(Node));
			children[i]->pos = current_node->pos;
			children[i]->parent = current_node;
		}

		children[0]->pos.x--;
		children[1]->pos.x++;
		children[2]->pos.y--;
		children[3]->pos.y++;

		// For each adjacent node
		for (int i = 0; i < 4; i++) {
			if (vector_has(closed_list, children[i], NULL)) {
				free(children[i]);
				continue;
			}
			if (map_get_collision(map, children[i]->pos.x, children[i]->pos.y, COLLISION_GHOST)) {
				free(children[i]);
				continue;
			}

			children[i]->g = current_node->g + 1;
			children[i]->h = -SDL_Point_Distance(&children[i]->pos, place_to_flee);
			children[i]->f = children[i]->g + children[i]->h;
			int index = 0;
			if (vector_has(open_list, children[i], &index)) {
				if (children[i]->g > open_list->list[index]->g) {
					free(children[i]);
					continue;
				}
			}
			vector_push_back(open_list, children[i]);
		}

		free(children);
	}
	vector_free(open_list);
	vector_free(closed_list);
}

void dbg_draw_a_star(SDL_Renderer *renderer, const SDL_Point *path, const int length, SDL_Point cam_offset) {
	for (int i = 0; i < length; i++) {
		SDL_Rect dst = { (path[i].x * 16) + cam_offset.x, (path[i].y * 16) + cam_offset.y, 16, 16 };
		SDL_RenderDrawRect(renderer, &dst);
	}
}