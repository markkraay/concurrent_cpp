#include <atomic>
#include <iostream>

int main() {
	std::atomic<int> a(0);
	// If true -> lock-free, actual atomic operations
	// False -> lock-based, atomic operations are implemented with locks
	std::cout << a.is_lock_free() << std::endl;
}