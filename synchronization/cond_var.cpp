#include <condition_variable>
#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <queue>

std::mutex mut;
std::queue<int> data_queue;
std::condition_variable data_cond;

void data_preparation_thread() {
	std::vector<int> data(10, 1);
	data.insert(data.begin(), 0);
	while (!data.empty()) {
		int unit = data.back();
		data.pop_back();
		std::lock_guard<std::mutex> lk(mut);
		data_queue.push(unit);
		data_cond.notify_one();
	}
}

void data_processing_thread() {
	while (true) {
		// Need unique_lock instead of lock_guard
		// because this thread must be able to unlock and relock
		// the mutex with the cond_var.
		std::unique_lock<std::mutex> lk(mut);
		// Lambda expresses the condition being waited for
		// wait...
		// 1. checks the condition
		// 2. If the condition is not satisfied, wait unlocks the mutex
		// and puts the thread in a sleeping state
		// 3. `notify_one` wakes the thread from its slumber, re-acquires the lock
		// and checks the condition again, returing if the cond is satisfied
		// 4. If not satisfied, thread unlocks mutex and resumes waiting
		data_cond.wait(lk, [] { return !data_queue.empty(); });
		int data = data_queue.front();
		data_queue.pop();
		lk.unlock();
		std::cout << "Received: " << data << std::endl;
		if (data == 0) break;
	}
}

int main() {
	std::thread t1(data_preparation_thread);
	std::thread t2(data_processing_thread);
	t1.join();
	t2.join();
}