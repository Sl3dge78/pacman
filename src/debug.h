#ifndef DEBUG_H
#define DEBUG_H

#include <stdlib.h>

void *DBG_malloc(size_t size, char *filename, int line);
void *DBG_calloc(size_t num, size_t size, char *filename, int line);
void *DBG_realloc(void *ptr, size_t new_size, char *filename, int line);
void DBG_free(void *ptr);
void DBG_dump_memory_leaks();

#define malloc(size) DBG_malloc(size, __FILE__, __LINE__)
#define calloc(num, size) DBG_calloc(num, size, __FILE__, __LINE__)
#define realloc(ptr, size) DBG_realloc(ptr, size, __FILE__, __LINE__)
#define free(ptr) DBG_free(ptr)

#endif