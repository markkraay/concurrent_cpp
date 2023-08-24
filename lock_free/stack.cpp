#include <atomic>
#include <thread>
#include <iostream>

template <typename T>
class Stack {
private:
	struct Node {
		std::shared_ptr<T> data;
		Node* next;

		Node(T const& data_) : data(new T(data_)) {}
	};
	std::atomic<Node*> head;
	std::atomic<Node*> to_be_deleted;
	std::atomic<unsigned> threads_in_pop;

	static void delete_nodes(Node* nodes) {
		while (nodes) {
			Node* next = nodes->next;
			delete nodes;
			nodes = next;
		}
	}

public:
	void push(T const& data) {
		Node* const new_node = new Node(data);
		new_node->next = head.load();
		// This is effectively waiting for the CAS to succeed.
		// So once head = new_node->next, head will be set to new_node and the thread will continue.
		while (!head.compare_exchange_weak(new_node->next, new_node));
	}

	std::shared_ptr<T> pop() {
		++threads_in_pop;
		Node* old_head = head.load();
		while (old_head && !head.compare_exchange_weak(old_head, old_head->next));
		std::shared_ptr<T> res;
		if (old_head) {
			res.swap(old_head->data);
		}
		Node* nodes_to_delete = to_be_deleted.load();
		if (!--threads_in_pop) {
			if (to_be_deleted.compare_exchange_strong(nodes_to_delete, nullptr)) {
				delete_nodes(nodes_to_delete);
			}
			delete old_head;
		} else if (old_head) {
			old_head->next = nodes_to_delete;
			while (!to_be_deleted.compare_exchange_weak(old_head->next, old_head));
		}
		return res;
	}
};

int main() {
	Stack<int> stack;
	stack.push(1);
	stack.push(2);
	stack.push(3);
}