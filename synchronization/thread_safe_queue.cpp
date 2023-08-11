#include <memory>
#include <chrono>
#include <thread>
#include <iostream>
#include <queue>
#include <condition_variable>
#include <mutex>

template <class T>
class queue {
private:
	mutable std::mutex mut;
	std::queue<T> data_queue;
	std::condition_variable data_cond;
public:
	queue() {}
	queue(const queue& other) {
		std::lock_guard<std::mutex> lk(other.mut);
		data_queue = other.data_queue;
	}
	queue& operator=(const queue&) = delete;

	void push(T new_value) {
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(new_value);
		data_cond.notify_one();
	}

	void wait_and_pop(T& value) {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this]{ return !data_queue.empty(); });
		value = data_queue.front();
		data_queue.pop();
	}

	std::shared_ptr<T> wait_and_pop() {
		std::unique_lock<std::mutex> lk(mut);
		data_cond.wait(lk, [this]{ return !data_queue.empty(); });
		std::shared_ptr<T> res(new T(data_queue.front()));
		data_queue.pop();
		return res;
	}

	bool try_pop(T& value) {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) return false;
		value = data_queue.front();
		data_queue.pop();
		return true;
	}

	std::shared_ptr<T> try_pop() {
		std::lock_guard<std::mutex> lk(mut);
		if (data_queue.empty()) return std::make_shared<T>(nullptr);
		std::shared_ptr<T> res(new T(data_queue.front()));
		data_queue.pop();
		return res;
	}

	bool empty() const {
		std::lock_guard<std::mutex> lk(mut);
		return data_queue.empty();
	}
};

queue<int> q;

void prepare_data() {
	int count = 0;
	while (count < 10) {
		q.push(count);
		std::this_thread::sleep_for(std::chrono::seconds(2));
		count++;
	}
}

void process_data() {
	int value;
	while (true) {
		q.wait_and_pop(value);
		std::cout << value << std::endl;
		if (value == 9) break;
	}
}

int main() {
	std::thread t1(prepare_data);
	std::thread t2(process_data);
	t1.join();
	t2.join();
}