#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "memory_manager.h"

// 手动分支预测优化
#define __plugin_unlikely(cond) __builtin_expect (!!(cond), 0)
#define __plugin_likely(cond) __builtin_expect (!!(cond), 1)

// 内存池内的内存对齐
#define __plugin_alignment   sizeof(unsigned long)

// 对齐指针
#define __plugin_align_ptr(p, a)                                                   \
    (char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

// 内存分配区大小
#define ARENA_SIZE 1 * 1024 * 1024


starry_sky::memory_manager* starry_sky::memory_manager::instance = NULL;
starry_sky::memory_manager::helper starry_sky::memory_manager::m_helper;

namespace starry_sky
{
	arena_node::arena_node(size_t size)
	{
		memory = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		// 如果 mmap 失败，发送关闭信号 SIGTERM
		if (__plugin_unlikely(memory == MAP_FAILED))
		{
			perror("mmap failed");
		}

		current_index = memory;
		memory_end = memory + size;

		capacity = size;
	}

	arena_node::~arena_node()
	{
		if (__plugin_likely(memory))
		{
			if (munmap(memory, capacity) == -1)
			{
				perror("munmap failed");
			}
			memory = nullptr;
			current_index = nullptr;
		}

	}

	fragment_node* arena_node::allot_memory(uint32_t size)
	{
		if (__plugin_unlikely(memory == nullptr || memory == MAP_FAILED))
		{
			return nullptr;
		}

		auto tmp = current_index;

		// 分配对齐后的fragment_node
		tmp = __plugin_align_ptr(tmp, __plugin_alignment);
		if (__plugin_unlikely(tmp + sizeof(fragment_node) > memory_end))
		{
			return nullptr;
		}
		fragment_node* node = reinterpret_cast<fragment_node*>(tmp);
		tmp = (char*)tmp + sizeof(fragment_node);

		// 分配对齐后的size
		tmp = __plugin_align_ptr(tmp, __plugin_alignment);
		if (__plugin_unlikely(tmp + size > memory_end))
		{
			return nullptr;
		}

		// 填充信息
		node->addr = tmp;
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

	void leisure_node_manager::add_node(fragment_node* node)
	{
		// 将节点插入到链表第一位
		int size = node->size;
		if (__plugin_unlikely(mp.count(size) == 0)) 
		{
			mp[size] = node;
			return;
		}
		node->next = mp[size];
		node->status = false;
		mp[size] = node;
	}

	fragment_node* leisure_node_manager::get_size(size_t size)
	{
		if (mp.count(size) == 0 || mp[size] == nullptr)
		{
			return nullptr;
		}

		fragment_node* node = nullptr;
		// 将mp里size大小的链表的第一个节点提取，返回
		if (__plugin_unlikely(mp[size] == nullptr))
		{
			return nullptr;
		}

		node = mp[size];
		node->status = true;
		mp[size] = node->next;

		return node;
	}

	memory_manager::~memory_manager()
	{
		close();
	}

	void memory_manager::close()
	{
		// 释放所有内存分配区
		int size = arena_list.size();
		for (auto node : arena_list) {
			printf("delete arena, size: %d\n", size--);
			delete node;
		}
		arena_list.clear();

		// 此时内存已释放，清空节点缓存
		leisure_manager.close();
	}

	void* memory_manager::allot_memory(size_t size)
	{
		// 先在空闲链表查找
		auto node = leisure_manager.get_size(size);
		if (node)
		{
			return node->addr;
		}

		// 在已有内存分配区里分配
		for (auto arena = arena_list.begin(); arena != arena_list.end(); arena++)
		{
			node = (*arena)->allot_memory(size);
			if (node)
			{
				arena_list.splice(arena_list.begin(), arena_list, arena);
				return node->addr;
			}
		}

		// 内存分配区都无法分配，则创建一个新的内存分配区
		auto arena = new arena_node(ARENA_SIZE);
		arena_list.push_front(arena);

		printf("create new arena, size: %d\n", arena_list.size());

		// 新内存分配区分配
		node = arena->allot_memory(size);

		return node->addr;
	}

	void memory_manager::free_memory(void* ptr)
	{
		// 将节点移到空闲节点管理模块里
		fragment_node* node = (fragment_node*)((char*)ptr - __plugin_align_ptr(sizeof(fragment_node), __plugin_alignment));
		if (__plugin_likely(node->status == true))
		{
			node->status = false;
			leisure_manager.add_node(node);
		}
	}
}
