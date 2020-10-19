#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>

void *DBG_malloc(size_t size, char *filename, int line);
void DBG_free(void *ptr);
void DBG_dump_memory_leaks();

#define malloc(x) DBG_malloc(x, __FILE__, __LINE__)
#define free(x) DGB_free(x)

#endif