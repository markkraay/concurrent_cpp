#include <map>
#include <mutex>
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <string>
#include <vector>

class dns_entry {};

class dns_cache {
	std::map<std::string, dns_entry> entries;
	std::shared_mutex entry_mutex;

public:
	dns_entry find_entry(const std::string& domain) {
		std::shared_lock lock(entry_mutex);
		return entries.at(domain);
	}

	void add_entry(const std::string& domain, dns_entry entry) {
		std::lock_guard lock(entry_mutex);
		entries.insert(std::make_pair(domain, entry));
	}

	void print() {
		std::shared_lock lock(entry_mutex);
		for (const auto& [name, _] : this->entries) {
			std::cout << name << std::endl;
		}
	}
};

int main() {
	dns_cache cache;
	std::vector<std::thread> threads(10);

	for (int i = 0; i < 10; i++) {
		dns_entry entry;
		threads[i] = std::thread(&dns_cache::add_entry, &cache, std::to_string(i), entry);
	}

	std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
	cache.print();
}