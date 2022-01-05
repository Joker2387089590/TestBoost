#pragma once
#include <Common.h>

namespace Net::Udp
{
using boost::asio::ip::udp;

struct Datagram : std::pair<udp::endpoint, std::vector<char>>
{
	using pair::pair;

	auto& endpoint() { return first; }
	auto& data() { return second; }

	auto& endpoint() const { return first; }
	auto& data() const { return second; }

	template<typename... Args>
	static std::shared_ptr<Datagram> make(Args&&... args)
	{
		return std::make_shared<Datagram>(std::forward<Args>(args)...);
	}

	static std::shared_ptr<Datagram> make(udp::endpoint e, std::string_view b)
	{
		return make(std::move(e), makeData(b));
	}
};

class Node :
    public std::enable_shared_from_this<Node>,
    protected udp::socket
{
public:
	using Ptr = std::shared_ptr<Node>;

public:
    [[nodiscard]] static Ptr make(Manager& manager);
    virtual ~Node() = default;
protected:
    Node(boost::asio::io_context& context);

public:
    using udp::socket::open;
    using udp::socket::bind;
    auto endpoint() const { return local_endpoint(); }

public:
	using AfterReceive = bool(Datagram& datagram);
	using ErrorReceive = bool(Datagram& incompleteDatagram,
							  const boost::system::error_code& error);
	void startRead(std::function<AfterReceive> afterReceive,
				   std::function<ErrorReceive> errroReceive = {});

public:
	using AfterWrite = void();
	using ErrorWrite = void(const boost::system::error_code& error,
							std::size_t byteSent);
	void startWrite(std::shared_ptr<Datagram> datagram,
					std::function<AfterWrite> afterWrite = {},
					std::function<ErrorWrite> errorWrite = {});
};

}
