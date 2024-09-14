#include <string.h>

#include "malloc.h"
#include "memory_manager.h"

thread_local static starry_sky::memory_manager manager;

void* plugin_malloc(size_t size)
{
    return manager.allot_memory(size);
}

void* plugin_calloc(size_t num, size_t size)
{
    void* tmp = manager.allot_memory(num * size);
    memset(tmp, 0, num * size);
    return tmp;
}

void* plugin_ralloc(void *ptr, size_t size)
{
    return nullptr;
}

void  plugin_free(void *ptr)
{
    return manager.free_memory(ptr);
}