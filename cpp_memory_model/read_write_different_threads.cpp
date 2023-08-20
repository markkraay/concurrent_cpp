#include <vector>
#include <chrono>
#include <atomic>
#include <iostream>
#include <thread>

std::vector<int> data;
std::atomic_bool data_ready(false);

void writer_thread() {
	data.push_back(42);
	data_ready = true;
}

void reader_thread() {
	// This loop is inefficient because it is constantly checking the value of data_ready
	while (!data_ready.load()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	std::cout << "Answer: " << data[0] << std::endl;
}

int main() {
	std::thread t1(writer_thread);
	std::thread t2(reader_thread);
	t1.join();
	t2.join();
}