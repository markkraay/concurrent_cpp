#include <memory>
#include <thread>
#include <iostream>
#include <string>

class BigObject {
private:
	std::string name;
public:	
	BigObject(std::string const& name_) : name(name_) {}

	void print() {
		std::cout << name << std::endl;
	}
};

void process_big_object(std::unique_ptr<BigObject> ptr) {
	ptr->print();
}

int main() {
	std::unique_ptr<BigObject> ptr(new BigObject("Mark"));
	std::thread t(process_big_object, std::move(ptr));
	t.join();
}