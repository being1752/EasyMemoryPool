#pragma once
#include <stdlib.h>

#ifdef _MSC_VER
#define __ATTR(s)
#else
#define __ATTR(s) __attribute__((s))
#endif


void* plugin_malloc(size_t size) __ATTR(malloc);
void* plugin_calloc(size_t num, size_t size) __ATTR(malloc);
void* plugin_ralloc(void *ptr, size_t size);
void  plugin_free(void *ptr);

