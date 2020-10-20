#include "debug.h"

#include <stdio.h>

#undef malloc
#undef calloc
#undef realloc
#undef free

typedef struct MemoryInfo {
	void *ptr;
	size_t size;
	char *file;
	int line;
} MemoryInfo;

typedef struct MemoryLeak {
	MemoryInfo info;
	struct MemoryLeak *next;
} MemoryLeak;

static MemoryLeak *array_start = NULL;
static MemoryLeak *array_end = NULL;

static void add_memory_info(void *ptr, size_t size, char *filename, int line) {
	MemoryLeak *leak = (MemoryLeak *)malloc(sizeof(MemoryLeak));

	leak->info.ptr = ptr;
	leak->info.size = size;
	leak->info.file = filename;
	leak->info.line = line;
	leak->next = NULL;

	if (array_start == NULL) {
		array_start = leak;
		array_end = leak;
	} else {
		array_end->next = leak;
		array_end = leak;
	}
}

static void delete_memory_info(void *ptr) {
	if (array_start->info.ptr == ptr) {
		MemoryLeak *leak = array_start;
		array_start = array_start->next;

		free(leak);
		return;
	}

	for (MemoryLeak *leak = array_start; leak != NULL; leak = leak->next) {
		MemoryLeak *next_leak = leak->next;
		if (next_leak == NULL) {
			printf("Couldn't find ptr to free\n");
			printf("Previous alloc: %s:%d\n", leak->info.file, leak->info.line);
			return;
		}
		if (next_leak->info.ptr == ptr) { // Si il faut supprimer le suivant
			if (array_end == next_leak) {
				leak->next = NULL;
				array_end = leak;
			} else {
				leak->next = next_leak->next;
			}
			free(next_leak);
			return;
		}
	}
}

static void clear_array() {
	MemoryLeak *leak = array_start;
	MemoryLeak *to_delete = array_start;

	while (leak != NULL) {
		leak = leak->next;
		free(to_delete);
		to_delete = leak;
	}
}

void *DBG_malloc(size_t size, char *filename, int line) {
	void *ptr = malloc(size);
	if (ptr != NULL) {
		add_memory_info(ptr, size, filename, line);
	}
	return ptr;
}

void *DBG_calloc(size_t num, size_t size, char *filename, int line) {
	void *ptr = calloc(num, size);
	if (ptr != NULL) {
		add_memory_info(ptr, num * size, filename, line);
	}
	return ptr;
}

void *DBG_realloc(void *ptr, size_t new_size, char *filename, int line) {
	void *new_ptr = realloc(ptr, new_size);
	if (new_ptr != NULL) {
		if (ptr != NULL)
			delete_memory_info(ptr);

		add_memory_info(new_ptr, new_size, filename, line);
	}
	return new_ptr;
}

void DBG_free(void *ptr) {
	delete_memory_info(ptr);
	free(ptr);
}

void DBG_dump_memory_leaks() {
	int count = 0;
	for (MemoryLeak *leak = array_start; leak != NULL; leak = leak->next) {
		if (count == 0)
			printf("\n\n"
				   "==================================================================\n"
				   "| Dumping memory leaks:\n"
				   "==================================================================\n");
		printf("| Address: %p | Size: %06d | File: %s:%d |\n", leak->info.ptr, leak->info.size, leak->info.file, leak->info.line);
		count++;
	}
	printf("==================================================================\n");
	printf("| Dump done, %d leak(s) found.\n", count);
	printf("==================================================================\n\n");
	clear_array();
}
