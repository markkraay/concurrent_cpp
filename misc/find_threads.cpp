#include <iostream>
#include <thread>

int main() {
	int nthreads = std::thread::hardware_concurrency();
	std::cout << "Threads: " << nthreads << std::endl;
}
