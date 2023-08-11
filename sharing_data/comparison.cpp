#include <string>
#include <iostream>
#include <mutex>

class BigObject {
private:
	std::string name;
public:
	BigObject(const std::string& name_) : name(name_) {}
	bool operator<(const BigObject& rhs) const {
		return name < rhs.name;
	}
};

class X {
private:
	BigObject some_detail;
	mutable std::mutex m;
public:
	X(const BigObject& sd) : some_detail(sd) {}
	friend bool operator<(const X& lhs, X const& rhs) {
		if (&lhs == &rhs) return false;
		// std::lock(lhs.m, rhs.m);
		// std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
		// std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);
		/// This could also use unique_lock, which relaxes the `lock_guard`
		/// Each unique_lock does not always own the mutex that it is associated with
		std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
		std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
		std::lock(lock_a, lock_b);
		return lhs.some_detail < rhs.some_detail;
	}
};

int main() {
	BigObject a("Hello");
	BigObject b("World");
	X x1(a);
	X x2(b);
	std::cout << (a < b) << std::endl;
}