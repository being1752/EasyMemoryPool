#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "memory_manager.h"

#define __plugin_unlikely(cond) __builtin_expect (!!(cond), 0)
#define __plugin_likely(cond) __builtin_expect (!!(cond), 1)

#define __plugin_alignment   sizeof(unsigned long)
#define __plugin_align_ptr(p, a)                                                   \
    (char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define ARENA_SIZE 1 * 1024 * 1024

namespace starry_sky
{

arena_node::arena_node(size_t size)
{
    memory = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // 如果 mmap 失败，发送关闭信号 SIGTERM
    if(__plugin_unlikely(memory == MAP_FAILED)) 
    {
        perror("mmap failed");
        kill(getpid(), SIGTERM);
    }

    current_index = memory;
    memory_end = memory + size;

    capacity = size;
}

arena_node::~arena_node()
{
    if(__plugin_likely(memory))
    {
        if (munmap(memory, capacity) == -1) 
        {
            perror("munmap failed");
            kill(getpid(), SIGTERM);
        }
        memory = nullptr;
        current_index = nullptr;
    }

}

fragment_node *arena_node::allot_memory(uint32_t size)
{
    if(__plugin_unlikely(memory == nullptr))
    {
        return nullptr;
    }

    auto tmp = current_index;

    // 分配对齐后的fragment_node
    tmp = __plugin_align_ptr(tmp, __plugin_alignment);
    if(__plugin_unlikely(tmp + sizeof(fragment_node) > memory_end))
    {
        return nullptr;
    }
    fragment_node* node = reinterpret_cast<fragment_node*>(tmp);
    tmp = (char*)tmp + sizeof(fragment_node);

    // 分配对齐后的size
    tmp = __plugin_align_ptr(tmp, __plugin_alignment);
    if(__plugin_unlikely(tmp + size > memory_end))
    {
        return nullptr;
    }

    // 填充信息
    node->addr = tmp;
    node->next = nullptr;
    node->size = size;
    node->status = true;

    tmp = (char*)tmp + size;
    current_index = tmp;

    return node;
}

leisure_node_manager::~leisure_node_manager()
{
    close();
}

void leisure_node_manager::close()
{
    mp.clear();
}

void leisure_node_manager::add_node(fragment_node *node)
{
    int size = node->size;
    if(mp.count(size) == 0)
    {
        mp[size] = node;
    }

    // 将节点插入到链表第一位
    node->next = mp[size];
    mp[size] = node;
}

fragment_node *leisure_node_manager::get_size(size_t size)
{
    if(__plugin_unlikely(mp.count(size) == 0))
    {
        return nullptr;
    }

    // 将mp里size大小的链表的第一个节点提取，返回
    auto node = mp[size];

    // 防止空节点
    if(__plugin_likely(node))
    {
        mp[size] = node->next;
        node->next = nullptr;
    }
    return node;
}

memory_manager::memory_manager()
{
}

memory_manager::~memory_manager()
{
    close();

//     if(__plugin_likely(leisure_manager))
//     {
//         free(leisure_manager);
//         leisure_manager = nullptr;
//     }
}

void memory_manager::close()
{
    // 释放所有内存分配区
    int size = arena_list.size();
	for (auto node : arena_list) {
 		delete node;
	}
    size = arena_list.size();
    arena_list.clear();
    size = arena_list.size();
    
    // 此时内存已释放，清空节点缓存
    leisure_manager.close();
    using_fragment_map.clear();
}

void *memory_manager::allot_memory(size_t size)
{
    // 先在空闲链表查找
    auto node = leisure_manager.get_size(size);
    if(node)
    {
        using_fragment_map[node->addr] = node;
        return node->addr;
    }
    
    // 在已有内存分配区里分配
    for(auto arena = arena_list.begin(); arena != arena_list.end(); arena++)
    {
        node = (*arena)->allot_memory(size);
        if (node) 
        {
			using_fragment_map[node->addr] = node;
			arena_list.splice(arena_list.begin(), arena_list, arena);
			return node->addr;
        }
    }
    
    // 内存分配区都无法分配，则创建一个新的内存分配区
    auto arena = new arena_node(ARENA_SIZE);
    int _size = arena_list.size();
    arena_list.push_front(arena);
    _size = arena_list.size();

    // 新内存分配区分配
    node = arena->allot_memory(size);
    using_fragment_map[node->addr] = node;

    return node->addr;
}

void memory_manager::free_memory(void *ptr)
{
    // 根据地址查找节点
    auto it = using_fragment_map.find(ptr);
    if(__plugin_unlikely(it == using_fragment_map.end()))
    {
        return;
    }

    // 将节点移到空闲节点管理模块里
    fragment_node* node = it->second;
    if(__plugin_likely(node->status == true))
    {
        node->status = false;
        using_fragment_map.erase(it);
        leisure_manager.add_node(node);
    }

    // 当前内存块没有在使用，说明流已经断掉，清空内存分配区
    if(__plugin_unlikely(using_fragment_map.empty()))
    {
        close();
    }
}

}
