#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <unordered_map>
#include <list>

namespace starry_sky
{

	// 内存块节点
	struct fragment_node
	{
		// 内存块地址
		void* addr = nullptr;
		// 是否被使用
		bool status;
		// 内存块大小
		uint32_t size;
		// 内存块下一个节点
		fragment_node* next = nullptr;

		fragment_node() = delete;
		fragment_node(u_char* _addr, uint16_t _size, bool _status = true) : addr(_addr), size(_size), status(_status) {}
		~fragment_node() = default;
	};

	// 内存分配区节点
	class arena_node
	{
	public:
		arena_node() = default;
		arena_node(size_t size);
		~arena_node();
		fragment_node* allot_memory(uint32_t size);

	private:
		// 内存分配区起始地址
		char* memory = nullptr;
		// 内存分配区结尾地址
		char* memory_end = nullptr;
		// 当前内存分配区分配到的位置
		char* current_index = nullptr;
		// 内存分配区容量
		uint32_t capacity;
	};

	// 空闲内存块管理模块
	class leisure_node_manager
	{
	public:
		leisure_node_manager() = default;
		~leisure_node_manager();
		void close();
		void add_node(fragment_node* node);
		fragment_node* get_size(size_t size);

	private:
		std::unordered_map<size_t, fragment_node*> mp;
	};

	// 内存分配管理模块
	class memory_manager
	{
	public:
		static memory_manager* get_instance() 
		{
			if (instance == nullptr) 
			{
				instance = new memory_manager();
			}
			return instance;
		}

		void  close();
		void* allot_memory(size_t size);
		void  free_memory(void* ptr);

	private:
		memory_manager() = default;
		~memory_manager();

		static void release_instance()
		{
			if (instance != nullptr)
			{
				auto* p = instance;
				instance = nullptr;
				delete p;
			}
		}

	private:
		static memory_manager* instance;
		// 内层分配区链表
		std::list<arena_node*> arena_list;
		// 空闲的内存块管理链表
		leisure_node_manager leisure_manager;

	private:
		class helper {
		public:
			helper() {
				memory_manager::get_instance();
			}
			~helper() {
				memory_manager::release_instance();
			}
		};
		static helper m_helper;
	};
}