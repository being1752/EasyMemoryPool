// EasyMemoryPool.cpp: 定义应用程序的入口点。
//
// /*
//#include "malloc.h"
#include <stdlib.h>
#include <iostream>
#include <list>
#include <sys/mman.h>
using namespace std;

int main()
{
	std::list<void*> ptrs;

	// 	while (1) {
	// 		for (int i = 0; i < 8000; i++) {
	// 			void* ptr = plugin_malloc(96);
	// 			ptrs.push_back(ptr);
	// 			ptr = plugin_malloc(1040);
	// 			ptrs.push_back(ptr);
	// 		}
	// 		for (auto ptr = ptrs.begin(); ptr != ptrs.end(); ptr++) {
	// 			plugin_free(*ptr);
	// 		}
	// 		ptrs.clear();
	// 	}

	// 	while (1) {
	// 		for (int i = 0; i < 3; i++) {
	// 			void* ptr = plugin_malloc(1024 * 1023);
	// 			ptrs.push_back(ptr);
	// 		}
	// 		for (auto ptr = ptrs.begin(); ptr != ptrs.end(); ptr++) {
	// 			plugin_free(*ptr);
	// 		}
	// 	}
	void* ptr;
	void* ptr1;
	void* ptr2;
	// 
	for (int i = 0; i < 1000; ++i) {
// 		ptr = plugin_malloc(1024 * 1023);
// 		ptr1 = plugin_malloc(1024 * 1023);
// 		plugin_free(ptr);
// 		ptr2 = plugin_malloc(1024 * 1023);
// 		plugin_free(ptr1);
// 		plugin_free(ptr2);

		ptr = malloc(1024 * 1023);
		ptr1 = malloc(1024 * 1023);
		free(ptr);
		ptr2 = malloc(1024 * 1023);
		free(ptr1);
		free(ptr2);

		// 		for (int i = 0; i < 3; i++) {
		// 			ptr = malloc(1024 * 1023);
		// 			//ptr = plugin_malloc(1024 * 1023);
		// 			//ptr1 = malloc(1024 * 1023);
		// 			//ptr2 = malloc(1024 * 1023);
		// 		 	ptrs.push_back(ptr);
		// 		}
		//  		for (auto ptr_ = ptrs.begin(); ptr_ != ptrs.end(); ptr_++) {
		//  			free(*ptr_);
		//  			//free(ptr1);
		//  			//free(ptr2);
		// 			//plugin_free(*ptr_);
		//  		}
		ptrs.clear();
	}
}
// */
