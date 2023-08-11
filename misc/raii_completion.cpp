/// Example program using RAII to manage a thread object
/// This will ensure that the thread is joined before going out of scope

#include <iostream>
#include <thread>

/// @brief RAII class to manage a thread object
/// @details This class will ensure that the thread is joined before going out of scope 
/// otherwise, the thread will run in detached mode (daemon thread), which might be problematic if the thread
/// references variables from the calling scope. 
class thread_guard {
	std::thread& t;
public:
	explicit thread_guard(std::thread& t_) : t(t_) {}
	~thread_guard() {
		if (t.joinable()) { // Checks that join has not been called before
			t.join();
		}
	}
	thread_guard(thread_guard const&) = delete;
	thread_guard& operator=(thread_guard const&) = delete;
};

void do_something_in_concurrent() {
	std::cout << "Do Something!" << std::endl;
}

void func() {
	std::cout << "Func!" << std::endl;
};

void f() {
	int some_local_state = 1;
	std::thread t(func);
	thread_guard g(t);
	do_something_in_concurrent();
}

int main() {
	f();
}