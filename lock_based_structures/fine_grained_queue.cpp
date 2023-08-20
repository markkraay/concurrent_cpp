#include <memory>
#include <mutex>
#include <iostream>

template <typename T>
class Queue {
private:
	struct Node {
		std::shared_ptr<T> data = nullptr;
		Node* next = nullptr;
	};
	std::mutex head_mutex;
	Node* head;
	std::mutex tail_mutex;
	Node* tail;
	std::condition_variable data_cond;

	Node* get_tail() {
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		return tail;
	}

	Node* pop_head() {
		Node* const old_head = head;
		head = old_head->next;
		return old_head;
	}

	Node* try_pop_head() {
		std::lock_guard<std::mutex> head_lock(head_mutex);
		if (head == get_tail()) {
			return nullptr;
		}
		return pop_head();
	}

	Node* try_pop_head(T& value) {
		std::lock_guard<std::mutex> head_lock(head_mutex);
		if (head == get_tail()) {
			return nullptr;
		}
		value = *head->data;
		return pop_head();
	}

	std::unique_lock<std::mutex> wait_for_data() {
		std::unique_lock<std::mutex> head_lock(head_mutex);
		data_cond.wait(head_lock, [&]{ return head != get_tail(); });
		return head_lock;
	}

	Node* wait_pop_head() {
		std::unique_lock<std::mutex> head_lock(wait_for_data());
		return pop_head();
	}

	Node* wait_pop_head(T& value) {
		std::unique_lock<std::mutex> head_lock(wait_for_data());
		value = *head->data;
		return pop_head();
	}

public:
	Queue() : head(new Node), tail(head) {}
	Queue(const Queue& other) = delete;
	Queue& operator=(const Queue& other) = delete;

	~Queue() {
		while (head) {
			Node* const old_head = head;
			head = old_head->next;
			delete old_head;
		}
	}

	std::shared_ptr<T> try_pop() {
		Node* const old_head = try_pop_head();
		if (!old_head) {
			return std::shared_ptr<T>();
		}
		std::shared_ptr<T> const res(old_head->data);
		delete old_head;
		return res;
	}

	bool try_pop(T& value) {
		Node* const old_head = try_pop_head(value);
		if (!old_head) {
			return false;
		}
		delete old_head;
		return true;
	}

	std::shared_ptr<T> wait_and_pop() {
		Node* const old_head = wait_pop_head();
		std::shared_ptr<T> const res(old_head->data);
		delete old_head;
		return res;
	}

	void wait_and_pop(T& value) {
		Node* const old_head = wait_pop_head(value);
		delete old_head;
	}

	void push(T new_value) {
		std::shared_ptr<T> new_head(new T(new_value));
		std::unique_ptr<Node> p(new Node);
		{
			std::lock_guard<std::mutex> tail_lock(tail_mutex);
			tail->data = new_head;
			tail->next = p.get();
			tail = p.release();
		}
		data_cond.notify_one();
	}

	void empty() {
		std::lock_guard<std::mutex> head_lock(head_mutex);
		return head == get_tail();
	}
};

int main() {
	Queue<int> q;
	q.push(1);
	q.push(2);
	q.push(3);
	q.push(4);
	std::shared_ptr<int> result;
	while ((result = q.try_pop()) != nullptr) {
		std::cout << *result << std::endl;
	}
}