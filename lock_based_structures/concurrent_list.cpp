#include <memory>
#include <thread>
#include <mutex>
#include <iostream>

template<typename T>
class ConcurrentList {
	struct Node {
		std::mutex m;
		std::shared_ptr<T> data = nullptr;
		Node* next = nullptr;

		Node() {}
		Node(T const& value) : data(new T(value)) {}
	};

	Node head;

public:
	ConcurrentList() {}
	ConcurrentList(ConcurrentList const& other) = delete;
	ConcurrentList& operator=(ConcurrentList const& other) = delete;
	~ConcurrentList() {
		remove_if([](T const&) { return true; });
	}

	void push_front(T const& value) {
		std::unique_ptr<Node> new_node(new Node(value));
		std::lock_guard<std::mutex> lk(head.m);
		new_node->next = head.next;
		head.next = new_node.release();
	}

	template<typename Function>
	void for_each(Function f) {
		Node* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (Node* const next = current->next) {
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			f(*next->data);
			current = next;
			lk = std::move(next_lk);
		}
	}

	template<typename Predicate>
	std::shared_ptr<T> find_first_if(Predicate p) {
		Node* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (Node* const next = current->next) {
			std::unique_lock<std::mutex> next_lk(next->m);
			lk.unlock();
			if (p(*next->data)) {
				return next->data;
			}
			current = next;
			lk = std::move(next_lk);
		}
	}

	template<typename Predicate>
	void remove_if(Predicate p) {
		Node* current = &head;
		std::unique_lock<std::mutex> lk(head.m);
		while (Node* const next = current->next) {
			std::unique_lock<std::mutex> next_lk(next->m);
			if (p(*next->data)) {
				current->next = next->next;
				next_lk.unlock();
				delete next;
			} else {
				lk.unlock();
				current = next;
				lk = std::move(next_lk);
			}
		}
	}
};

void worker(ConcurrentList<int>& list, int i) {
	list.push_front(i);
	list.for_each([](int i) { std::cout << i << std::endl; });
}

int main() {
	ConcurrentList<int> list;
	for (int i = 0; i < 20; i++) {
		worker(list, i);
	}
}