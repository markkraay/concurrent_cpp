#include <mutex>
#include <memory>
#include <iostream>
#include <thread>

class some_resource {
private:
	int data = 5;

public:
	static int count;

	some_resource() {
		count++;
	}

	void do_something() {
		std::cout << data << std::endl;
	}
};

int some_resource::count = 0;

std::shared_ptr<some_resource> resource_ptr;
std::once_flag resource_flag;

void init_resource() {
	resource_ptr.reset(new some_resource);
}

void foo() {
	std::call_once(resource_flag, init_resource);
	resource_ptr->do_something();
	std::cout << some_resource::count << std::endl;
}

int main() {
	std::thread t1(foo);
	std::thread t2(foo);
	t1.join();
	t2.join();
}