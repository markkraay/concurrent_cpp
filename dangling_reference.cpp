#include <iostream>
#include <thread>

struct func {
	int& i;
	func(int& i_) : i(i_) {}
	void operator()() {
		for (unsigned j = 0; j < 1000000; ++j) {
			std::cout << i << std::endl; // Potential access to dangling reference
		}
	}
};

int main() {
	{
		int some_local_state = 0;
		func my_func(some_local_state);
		std::thread t(my_func); // Potential dangling reference
		// This can be remeedied by joining the thread before the variable goes out of scope
	}
}