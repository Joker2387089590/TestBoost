#include <iostream>
#include <set>
#include "Udp.h"

using namespace Net::Udp;
using namespace std::literals;
using boost::system::error_code;
using boost::asio::ip::address_v4;

int main()
{
    const udp::endpoint readEndpoint{address_v4::any(), 50000};
	std::cout << readEndpoint.address().to_v4().to_string() << std::endl;

    Net::Manager manager;

	auto nodeRead = Node::make(manager);
    nodeRead->open(udp::v4());
    nodeRead->bind(readEndpoint);

	std::promise<void> waiter;
    std::set<udp::endpoint> users;
    nodeRead->startRead([&, i = 20](Datagram& datagram) mutable
	{
        auto& [endpoint, data] = datagram; []{}();
		std::cout << "Sender: " << endpoint.address().to_string() << ':' << endpoint.port() << '\n';
        std::cout << "Receive Data: " << Net::toView(data) << '\n';

        auto msg = "New user: " + endpoint.address().to_string();
        auto tmp = users.extract(endpoint);
        for(auto& other : users)
            nodeRead->startWrite(Datagram::make(other, std::string_view(msg)));
        if(tmp)
            users.insert(std::move(tmp));
        else
            users.insert(endpoint);

		if(--i == 0) waiter.set_value();
		return true;
	},
	[](Datagram& incompleteDatagram, const error_code& error)
	{
		auto& [endpoint, data] = incompleteDatagram;
		std::cerr << "Sender: " << endpoint.address().to_string() << ':' << endpoint.port() << '\n';
		std::cerr << "Error: " << error.message() << '\n';
        std::cerr << "Incomplete Data: " << Net::toView(data) << '\n';
		return true;
	});
	waiter.get_future().get();
    return 0;
}
