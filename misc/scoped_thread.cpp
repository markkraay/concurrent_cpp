#include <thread>
#include <iostream>
#include <stdexcept>

/// @brief A class that manages the lifetime of a thread,
/// mainly for ensuring that a thread is joined before going out of scope
class scoped_thread {
	std::thread t;

public:
	explicit scoped_thread(std::thread t_) : t(std::move(t_)) {
		if (!t.joinable()) {
			throw std::logic_error("No thread");
		}
	}

	~scoped_thread() {
		t.join();
	}

	scoped_thread(scoped_thread const&) = delete;
	scoped_thread& operator=(scoped_thread const&) = delete;
};

void func(int i) {
	std::cout << "func(" << i << ")" << std::endl;
}

void f() {
	int some_local_state = 0;
	scoped_thread t(std::thread(func, some_local_state));
	std::cout << "f()" << std::endl;
}

int main() {
	f();
}