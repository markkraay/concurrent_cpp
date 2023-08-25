#include <iostream>
#include <atomic>
#include <memory>

template<typename T>
class Stack {
private:
	struct Node;

	struct CountedNode
}
// First, it is wise to check if shared_ptr's are lock-free. In my case (which is likely), they are not.
// So, we have to do a workaround.
struct Dummy {};

int main() {
	std::shared_ptr<Dummy> a(new Dummy);
	std::cout << std::atomic_is_lock_free(&a) << std::endl;
}