#include <iostream>
#include "Udp.h"

using namespace Udp;
using namespace std::literals;
using boost::system::error_code;
using boost::asio::ip::address_v4;

constexpr int port = 50000;

const udp::endpoint readEndpoint{address_v4::from_string("192.168.1.243"), 52121 };

int main(int argc, char* argv[])
{
	Manager manager;

	if(argc < 2)
	{
		std::cout << "No host!\n";
		return 1;
	}

	udp::resolver resolver(manager.context());
	const auto endpoint = [&]
	{
		for(auto&& entry : resolver.resolve(argv[1], std::to_string(port)))
		{
			auto endpoint = entry.endpoint();
			if(endpoint.protocol() == udp::v4()) return endpoint;
		}
		std::cout << "Invalid host!\n";
		return udp::endpoint{};
	}();

	auto nodeWrite = Node::make(manager);
	nodeWrite->socket().open(udp::v4());

	boost::asio::high_resolution_timer timer(manager.context());
	std::promise<void> waiter;
	auto send = [&waiter, &timer, &endpoint, nodeWrite](auto send, int i = 10) -> void
	{
		if(i < 0) return waiter.set_value();
		auto callback = [=](const boost::system::error_code&)
		{
			nodeWrite->startWrite(Datagram::make(endpoint, "Axf"sv));
			send(send, i - 1);
		};
		timer.expires_after(1s);
		timer.async_wait(callback);
	};
	send(send);
	waiter.get_future().get();

	return 0;
}
