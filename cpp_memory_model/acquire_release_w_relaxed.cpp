#include <thread>
#include <atomic>

std::atomic_bool x,y;
std::atomic_int z;

void write_x_then_y() {
	x.store(true, std::memory_order_relaxed);
	y.store(true, std::memory_order_release);
}

void read_y_then_x() {
	while (!y.load(std::memory_order_acquire));
	if (x.load(std::memory_order_relaxed))
		++z;
}

int main() {
	x = false;
	y = false;
	z = 0;
	std::thread a(write_x_then_y);
	std::thread b(read_y_then_x);
	a.join();
	b.join();
	// Cannot fail
	assert(z.load() != 0);
}