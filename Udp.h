#pragma once
#include <boost/asio.hpp>

namespace Udp
{

class Manager
{
public:
	explicit Manager();
	virtual ~Manager();
	auto& context() { return m_context; }

protected:
	virtual void ioErrorOccured(const boost::system::error_code& error);

private:
	boost::asio::io_context m_context;
	std::thread m_ioThread;
};

inline std::vector<char> makeData(std::string_view buffer)
{
	return std::vector<char>(buffer.begin(), buffer.end());
}

inline std::string_view toView(const std::vector<char>& buffer)
{
	return std::string_view(buffer.begin(), buffer.end());
}

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

class Node : public std::enable_shared_from_this<Node>
{
public:
	using Ptr = std::shared_ptr<Node>;
	[[nodiscard]] static Ptr make(Manager& manager);
	// [[nodiscard]] static Ptr open(Manager& manager, udp::endpoint e = {});

protected:
	Node(boost::asio::io_context& context);

public:
	bool bind(const udp::endpoint& endpoint);
	auto endpoint() const { return m_socket.local_endpoint(); }

	auto& socket() const { return m_socket; }
	auto& socket() { return m_socket; }

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

private:
	udp::socket m_socket;
};

}
