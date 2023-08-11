#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <iostream>

struct file_contents {
	std::string content;
};

file_contents download_file(const std::string& filename) {
	std::this_thread::sleep_for(std::chrono::seconds(5));
	return file_contents{"File: " + filename};
}

std::future<file_contents> download_file_in_background(const std::string& filename) {
	std::packaged_task<file_contents()> task(std::bind(download_file, filename));
	std::future<file_contents> res(task.get_future());
	std::thread(std::move(task));
	return res;
}

int main() {
	std::future<file_contents> content = download_file_in_background("img.png");
	std::cout << content.get().content << std::endl;
}