#include <iostream>
#include "Udp.h"

using namespace Udp;
using namespace std::literals;
using boost::system::error_code;
using boost::asio::ip::address_v4;

int main()
{
	const udp::endpoint readEndpoint{address_v4::any(), 50000};
	std::cout << readEndpoint.address().to_v4().to_string() << std::endl;

	Manager manager;

	auto nodeRead = Node::make(manager);
	nodeRead->socket().open(udp::v4());
	nodeRead->socket().bind(readEndpoint);

	std::promise<void> waiter;
	int i = 10;
	nodeRead->startRead([&](Datagram& datagram)
	{
		auto& [endpoint, data] = datagram;
		std::cout << "Sender: " << endpoint.address().to_string() << ':' << endpoint.port() << '\n';
		std::cout << "Receive Data: " << toView(data) << '\n';
		if(--i == 0) waiter.set_value();
		return true;
	},
	[](Datagram& incompleteDatagram, const error_code& error)
	{
		auto& [endpoint, data] = incompleteDatagram;
		std::cerr << "Sender: " << endpoint.address().to_string() << ':' << endpoint.port() << '\n';
		std::cerr << "Error: " << error.message() << '\n';
		std::cerr << "Incomplete Data: " << toView(data) << '\n';
		return true;
	});
	waiter.get_future().get();
	return 0;
}
