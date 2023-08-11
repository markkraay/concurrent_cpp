#include <mutex>
#include <thread>
#include <iostream>
#include <thread>

struct Handle {};

class Connection {
public:
	void send_data(const std::string& data) {
		std::cout << "Sent: " << data << std::endl;
	}

	std::string receive_data() {
		return "Received: Data" ;
	}
};

class X {
private:
	Handle handle;
	Connection connection;
	std::once_flag connection_init_flag;

	void open_connection() {
		this->connection = Connection();
		connections++;
	}

public:
	static int connections;
	X() = default;

	void send_data(const std::string& data) {
		std::call_once(connection_init_flag, &X::open_connection, this);
		connection.send_data(data);
	}

	std::string receive_data() {
		std::call_once(connection_init_flag, &X::open_connection, this);
		return connection.receive_data();
	}
};

// Will tell us how many connections we have spawned
int X::connections = 0;

void f() {
	X x = X();
	x.send_data("Hello!");
	std::cout << x.receive_data() << std::endl;
}

int main() {
	std::thread t1(f);
	std::thread t2(f);
	t1.join();
	t2.join();
	/// Should be 2, since we open the connection for each class
	std::cout << X::connections << std::endl;
}