// EasyMemoryPool.cpp: 定义应用程序的入口点。
//
// /*
#include "malloc.h"
#include <iostream>
#include <list>
#include <sys/mman.h>
using namespace std;

int main()
{
	int* a = new int;
	int* b = new int;
	bool tr = (&a < &b);
	std::cout << "sdafasf";
	std::cout << tr;

	std::list<void*> ptrs;
	/*
	while (1) {
		for (int i = 0; i < 8000; i++) {
			void* ptr = plugin_malloc(96);
			ptrs.push_back(ptr);
			ptr = plugin_malloc(1040);
			ptrs.push_back(ptr);
		}
		for (auto ptr = ptrs.begin(); ptr != ptrs.end(); ptr++) {
			plugin_free(*ptr);
		}
	}
	*/
	while (1) {
		for (int i = 0; i < 3; i++) {
			void* ptr = plugin_malloc(1024 * 1023);
			ptrs.push_back(ptr);
		}
		for (auto ptr = ptrs.begin(); ptr != ptrs.end(); ptr++) {
			plugin_free(*ptr);
		}
	}
}
// */
