#include <list>
#include <type_traits>
#include <thread>
#include <iostream>
#include <future>


template <typename F, typename A>
std::future<std::result_of<F(A&&)>::type> spawn_task(F&& f, A&& a) {
	typedef std::result_of<F(A&&)>::type result_type;
	std::packaged_task<result_type(A&&)> task(std::move(f));
	std::future<result_type> res(task.get_future());
	std::thread(std::move(task), std::move(a));
	return res;
}

template <typename T>
std::list<T> parallel_quick_sort(std::list<T> input) {
	if (input.empty()) {
		return input;
	}

	std::list<T> result;
	result.splice(result.begin(), input, input.begin());
	T const& partition_val = *result.begin();

	auto divide_point = std::partition(input.begin(), input.end(), [&](T const& t) {
		return t < partition_val; 
	});

	std::list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	std::future<std::list<T>> new_lower(
		spawn_task(&parallel_quick_sort<T>, std::move(lower_part))
	);

	std::list<T> new_higher(parallel_quick_sort(std::move(input)));

	result.splice(result.end(), new_higher);
	result.splice(result.begin(), new_lower.get());
	return result;
}

int main() {
	std::list<int> items = {9, 1, 12, 4, 5, 3, 12, 99};
	items = parallel_quick_sort(std::move(items));
	for (int i : items) {
		std::cout << i << std::endl;
	}
}