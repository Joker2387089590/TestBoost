#include <iostream>
#include "Udp.h"

using namespace Udp;
using namespace std::literals;
using boost::system::error_code;
using boost::asio::ip::address_v4;

const udp::endpoint readEndpoint{address_v4::loopback(), 52121};

void test()
{
	Manager manager;
	auto nodeWrite = Node::make(manager);
	nodeWrite->socket().open(udp::v4());

	boost::asio::high_resolution_timer timer(manager.context());
	std::promise<void> waiter;
	auto send = [&waiter, &timer, nodeWrite](auto send, int i = 10) -> void
	{
		if(i < 0) return waiter.set_value();
		auto callback = [=](const boost::system::error_code&)
		{
			nodeWrite->startWrite(Datagram::make(readEndpoint, "Axf"sv));
			send(send, i - 1);
		};
		timer.expires_after(1s);
		timer.async_wait(callback);
	};
	send(send);
	waiter.get_future().get();
}
