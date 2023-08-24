#include <atomic>
#include <thread>

unsigned const max_hazard_pointers = 100;
struct HazardPointer {
	std::atomic<std::thread::id> id;
	std::atomic<void*> pointer;
};

HazardPointer hazard_pointers[max_hazard_pointers];

class HPOwner {
	HazardPointer* hp;
public:
	HPOwner(HPOwner const&) = delete;
	HPOwner operator=(HPOwner const&) = delete;

	HPOwner(): hp(nullptr)  {
		for (unsigned i = 0; i < max_hazard_pointers; i++) {
			std::thread::id old_id;
			if (hazard_pointers[i].id.compare_exchange_strong(old_id, std::this_thread::get_id())) {
				hp = &hazard_pointers[i];
				break;
			}
		}
		if (!hp) {
			throw std::runtime_error("No hazard pointers available");
		}
	}

	std::atomic<void*>& get_pointer() {
		return hp->pointer;
	}

	~HPOwner() {
		hp->pointer.store(nullptr);
		hp->id.store(std::thread::id());
	}
};

std::atomic<void*>& get_hazard_pointer_for_current_thread() {
	thread_local static HPOwner hazard;
	return hazard.get_pointer();
}

bool outstanding_hazard_pointers_for(void* p) {
	for (unsigned i = 0; i < max_hazard_pointers; i++) {
		if (hazard_pointers[i].pointer.load() == p) return true;
	}
	return false;
}

template <typename T>
void do_delete(void* p) {
	delete static_cast<T*>(p);
}

struct data_to_reclaim {
	void* data;
	std::function<void(void*)> deleter;
	data_to_reclaim* next = nullptr;

	template<typename T>
	data_to_reclaim(T* p) : data(p), deleter(&do_delete<T>) {}
	~data_to_reclaim() { 
		deleter(data);
	}
};

std::atomic<data_to_reclaim*> nodes_to_reclaim;

template<typename T>
void reclaim_later(T* data) {
	add_to_reclaim_list(new data_to_reclaim(data));
}

void delete_nodes_with_no_hazards() {
	data_to_reclaim* current = nodes_to_reclaim.exchange(nullptr);
	while (current) {
		data_to_reclaim* const next = current->next;
		if (!outstanding_hazard_pointers_for(current->data)) {
			delete current;
		} else {
			add_to_reclaim_list(current);
		}
		current = next;
	}
}

void add_to_reclaim_list(data_to_reclaim* node) {
	node->next = nodes_to_reclaim.load();
	while (!nodes_to_reclaim.compare_exchange_weak(node->next, node));
}

template <typename T>
class HazardPointerStack {
private:
	struct Node {
		std::shared_ptr<T> data;
		Node* next;

		Node(T const& data_) : data(new T(data_)) {}
	};
	std::atomic<Node*> head;

public:
	void push(T const& data) {
		Node* const new_node = new Node(data);
		new_node->next = head.load();
		// This is effectively waiting for the CAS to succeed.
		// So once head = new_node->next, head will be set to new_node and the thread will continue.
		while (!head.compare_exchange_weak(new_node->next, new_node));
	}
 
	std::shared_ptr<T> pop() {
		std::atomic<Node*> hp = get_hazard_pointer_for_current_thread();
		Node* old_head = head.load();
		do {
			Node* temp;
			do {
				temp = old_head;
				hp.store(old_head);
				old_head = head.load();
			} while (old_head != temp);
		} while (old_head && !head.compare_exchange_strong(old_head, old_head->next));
		hp.store(nullptr);
		std::shared_ptr<T> res;
		if (old_head) {
			res.swap(old_head->data);
			if (outstanding_hazard_pointers_for(old_head)) {
				reclaim_later(old_head);
			} else {
				delete old_head;
			}
			delete_nodes_with_no_hazards();
		}
		return res;
	}
};

int main() {
	HazardPointerStack<int> s;
	s.push(3);
}