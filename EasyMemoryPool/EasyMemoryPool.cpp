// EasyMemoryPool.cpp: 定义应用程序的入口点。
//
// /*
#include "memory_manager.h"
using namespace std;
using namespace starry_sky;


#include <deque>    // For std::deque
#include <list>     // For std::list
#include <memory>   // For std::shared_ptr
#include <iostream> // For std::cout

// 测试类
class TestClass {
public:
	uint8_t* buffer;  // 指向分配的内存块

	// 构造函数
	TestClass(size_t size) {
		buffer = reinterpret_cast<uint8_t*>(memory_manager::get_instance()->allot_memory(size));
		std::cout << "TestClass constructed with buffer at: " << reinterpret_cast<void*>(buffer) << std::endl;
	}

	// 析构函数
	~TestClass() {
		if (buffer) {
			memory_manager::get_instance()->free_memory(buffer);
			std::cout << "TestClass  destructed  and buffer at: " << reinterpret_cast<void*>(buffer) << std::endl;
		}
	}
	static void* operator new(size_t size)
	{
		void* p = memory_manager::get_instance()->allot_memory(size);
		if (!p) {
			throw std::bad_alloc();  // 分配失败抛出异常
		}
		return p;
	}
	static void operator delete(void* p)
	{
		memory_manager::get_instance()->free_memory(p);
	}
};

auto custom_deleter = [](TestClass* ptr) {
	delete ptr;  // 调用我们自定义的 delete 操作符
	};

int main() {
	// 创建一个双端队列，最多保存3个list
	std::deque<std::list<std::shared_ptr<TestClass>>> queue;

	size_t size2 = 1040; // 用于 TestClass 中 buffer 的大小

		// 向队列中插入3个list，每个list装10000个共享指针的 TestClass
		for (int n = 0; n < 3; ++n) {
			std::list<std::shared_ptr<TestClass>> newList;
			for (int i = 0; i < 10000; ++i) {
				newList.push_back(std::shared_ptr<TestClass>(new TestClass(size2)));
			}
			queue.push_back(newList);  // 插入到队尾
			std::cout << "List " << n + 1 << " inserted into the queue.\n";
		}

	while (1) {

		// 模拟插入更多的list，每次插入新list时，若队列满了（超过3个list），弹出队首的list
		for (int round = 0; round < 3; ++round) {  // 模拟多次插入
			// 若队列已满3个list，则弹出队首list，释放资源
			if (queue.size() == 3) {
				std::cout << "Queue is full. Removing front list...\n";
				queue.pop_front();  // 弹出队首list并释放其所有资源
				std::cout << "Front list removed.\n";
			}

			// 插入新的list
			std::list<std::shared_ptr<TestClass>> newList;
			for (int i = 0; i < 10000; ++i) {
				newList.push_back(std::shared_ptr<TestClass>(new TestClass(size2)));
			}
			queue.push_back(newList);  // 插入到队尾
			std::cout << "New list inserted into the queue.\n";
		}

		static int count = 0;
		count++;
		if (count == 5) {
			count = 0;
			while (queue.size() > 0) {
				queue.pop_front();
			}
			memory_manager::get_instance()->close();
			for (int n = 0; n < 3; ++n) {
				std::list<std::shared_ptr<TestClass>> newList;
				for (int i = 0; i < 10000; ++i) {
					newList.push_back(std::shared_ptr<TestClass>(new TestClass(size2)));
				}
				queue.push_back(newList);  // 插入到队尾
				std::cout << "List " << n + 1 << " inserted into the queue.\n";
			}
		}

	}
	return 0;
}