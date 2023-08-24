#include <functional>
#include <map>
#include <iostream>
#include <vector>
#include <list>
#include <shared_mutex>

template<typename Key, typename Value, typename Hash=std::hash<Key>>
class LookupTable {
private:
	using BucketValue = std::pair<Key, Value>;
	using BucketData = std::list<BucketValue>;
	using BucketIterator = typename BucketData::iterator;
	struct BucketType {
		BucketData data;
		std::shared_mutex mutex;

		BucketIterator find_entry_for(Key const& key) {
			return std::find_if(data.begin(), data.end(), [&](BucketValue const& item) {
				return item.first == key;
			});
		}
	};

	std::vector<BucketType*> buckets;
	Hash hasher;

	BucketType& get_bucket(Key const& key) const {
		std::size_t const bucket_index = hasher(key) % buckets.size();
		return *buckets[bucket_index];
	}

public:
	using KeyType = Key;
	using ValueType = Value;
	using HashType = Hash;

	LookupTable(unsigned num_buckets=19, Hash const& hasher_=Hash()) : buckets(num_buckets), hasher(hasher_) {
		for (unsigned i = 0; i < num_buckets; i++) {
			buckets[i] = new BucketType;
		}
	}

	LookupTable(LookupTable const& other) = delete;
	LookupTable& operator=(LookupTable const& other) = delete;

	~LookupTable() {
		for (unsigned i = 0; i < buckets.size(); i++) {
			delete buckets[i];
		}
	}

	Value value_for(Key const& key, Value const& default_value=Value()) {
		BucketType& bucket = get_bucket(key);
		std::shared_lock<std::shared_mutex> lock(bucket.mutex);
		BucketIterator const found_entry = bucket.find_entry_for(key);
		return found_entry == bucket.data.end() ? default_value : found_entry->second;
	}

	void add_or_update_mapping(Key const& key, Value const& value) {
		BucketType& bucket = get_bucket(key);
		std::unique_lock<std::shared_mutex> lock(bucket.mutex);
		BucketIterator const found_entry = bucket.find_entry_for(key);
		if (found_entry == bucket.data.end()) {
			bucket.data.push_back(BucketValue(key, value));
		} else {
			found_entry->second = value;
		}
	}

	void remove_mapping(Key const& key) {
		BucketType& bucket = get_bucket(key);
		std::unique_lock<std::shared_mutex> lock(bucket.mutex);
		BucketIterator const found_entry = bucket.find_entry_for(key);
		if (found_entry != bucket.data.end()) {
			bucket.data.erase(found_entry);
		}
	}

	std::map<Key, Value> get_map() const {
		std::vector<std::unique_lock<std::shared_mutex>> locks;
		for (unsigned i = 0; i < buckets.size(); i++) {
			locks.push_back(std::unique_lock<std::shared_mutex>(buckets[i].mutex));
		}
		std::map<Key, Value> res;
		for (unsigned int i = 0; i < buckets.size(); i++) {
			for (BucketIterator it = buckets[i].data.begin(); it != buckets[i].data.end(); it++) {
				res.insert(*it);
			}
		}
		return res;
	}
};

int main() {
	LookupTable<std::string, int> table;
	table.add_or_update_mapping("John", 123456);
	table.add_or_update_mapping("Mark", 41234);
	std::cout << table.value_for("John") << std::endl;
	std::cout << table.value_for("Mark") << std::endl;
}