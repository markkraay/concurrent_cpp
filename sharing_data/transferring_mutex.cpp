#include <mutex>
#include <thread>
#include <iostream>

std::mutex some_mutex;
int value = 0;

void prepare_data() {
	value += 1;
}

std::unique_lock<std::mutex> get_lock() {
	extern std::mutex some_mutex;
	std::unique_lock<std::mutex> lk(some_mutex);
	prepare_data();
	return lk;
}

void do_something() {
	std::cout << value << std::endl;
}

void process_data() {
	std::unique_lock<std::mutex> lk(get_lock());
	do_something();
}

int main() {
	std::thread t1(do_something);
	std::thread t2(process_data);
	t1.join();
	t2.join();
}