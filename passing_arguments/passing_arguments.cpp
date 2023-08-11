#include <string>
#include <thread>
#include <iostream>

/// Bear in mind that the thread constructor 
/// copies the arguments to internal storage.
void f(int i, std::string const& s) {
	std::cout << "i = " << i << ", s = " << s << std::endl;
}

void oops(int some_param) {
	char buffer[1024];
	sprintf(buffer, "%i", some_param);
	// This will lead to a dangling reference 
	// because the `buffer` will go out of scope 
	// when `oops` returns.
	std::thread t(f, 3, buffer);  
}

/// Another example
/// void update_data_for_widget(widget_id, w, widget_data& data);
/// 
/// void oops_again(widget_id w) {
/// 	widget_data data;
///   std::thread t(update_data_for_widget, w, data); 
///   display_status();
///   t.join();
///	  process_widget_data(data);
/// }
/// This is problematic because the thread will copy `w`
/// and `data` to internal storage, and then pass `data` by
/// reference to `update_data_for_widget`. 
/// Therefore any changes made to `data` will be lost when the thread
/// finishes execution.
/// The solution is to use `std::ref` to wrap the arguments:
/// std::thread t(update_data_for_widget, w, std::ref(data));

int main() {
	std::thread t(f, 3, "hello");
	t.join();
	oops(51);
}