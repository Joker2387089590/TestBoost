#include <iostream>
#include "Udp.h"

using namespace Net::Udp;
using namespace std::literals;
using boost::system::error_code;
using boost::asio::ip::address_v4;

constexpr int port = 50000;

int main(int argc, char* argv[])
{
    Net::Manager manager;

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

    std::cout << endpoint.address().to_string() << std::endl;

	auto nodeWrite = Node::make(manager);
    nodeWrite->open(udp::v4());

    nodeWrite->startRead([](Datagram& datagram)
    {
        auto& [endpoint, data] = datagram; []{}();
        std::cout << "Sender: " << endpoint.address().to_string() << ':' << endpoint.port() << '\n';
        std::cout << "Receive Data: " << Net::toView(data) << '\n';
        return true;
    });


    for(int i = 0; i < 10; ++i)
    {
        auto timer = std::make_shared<boost::asio::high_resolution_timer>(manager.context());
        timer->expires_after(i * 1s);
        timer->async_wait([&, timer](const boost::system::error_code&)
        {
            nodeWrite->startWrite(Datagram::make(endpoint, "Axf"sv));
            std::cout << "Write One\n";
        });
    }

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
