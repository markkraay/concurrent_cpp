#include <iostream>
#include <thread>

class X {
public:
	void do_lengthy_work() {
		std::cout << "Hello from thread" << std::endl;
	}
};

int main() {
	X my_x;
	std::thread t(&X::do_lengthy_work, &my_x);
	t.join();
}