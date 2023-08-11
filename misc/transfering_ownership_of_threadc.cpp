#include <thread>
#include <iostream>

void some_function() {
	std::cout << "some_function" << std::endl;
}

void some_other_function() {
	std::cout << "some_other_function" << std::endl;
}

void example1() {
	std::thread t1(some_function);
	std::thread t2 = std::move(t1);
	t1 = std::thread(some_other_function);
	t1.join();
	t2.join();
}

// Returning a thread from a function
std::thread example2() {
	void some_function();
	return std::thread(some_function);
}

int main() {
	example1();
	std::thread t = example2();
	t.join();
}