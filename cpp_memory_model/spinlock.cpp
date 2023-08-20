#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>

class spinlock_mutex {
	std::atomic_flag flag;
public:
	spinlock_mutex() : flag(ATOMIC_FLAG_INIT) {}

	void lock() {
		while (flag.test_and_set(std::memory_order_acquire));
	}

	void unlock() {
		flag.clear(std::memory_order_release);
	}
};

int a = 0;
spinlock_mutex m;

int f() {
	for (int i = 0; i < 10; i++) {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::lock_guard<spinlock_mutex> lk(m);
		a++;
		std::cout << a << std::endl;
	}
}

int main() {
	std::thread t1(f);
	std::thread t2(f);
	t1.join();
	t2.join();
}