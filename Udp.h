#pragma once
#include <boost/asio.hpp>
namespace Udp
{
	using boost::asio::ip::udp;

	struct Datagram : std::pair<udp::endpoint, std::vector<char>>
	{
		using pair::pair;

		auto& endpoint() { return first; }
		auto& data() { return second; }

		auto& endpoint() const { return first; }
		auto& data() const { return second; }
	};

	class OperationHandle
	{
	public:
		bool isRunning() const;
		bool isEnabled() const;
		void cancel();
	private:
		std::atomic_bool m_isRunning;
		std::atomic_bool m_enable;
	};

	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		Node(boost::asio::io_context& context);
		bool bind(const udp::endpoint& endpoint);

	public:
		using AfterReceive = bool(Datagram& datagram);
		using ErrorReceive = bool(Datagram& incompleteDatagram,
								  const boost::system::error_code& error);
		void startRead(std::function<AfterReceive> processReceive,
					   std::function<ErrorReceive> processError = {});

	public:
		using AfterWrite = bool();
		using ErrorWrite = bool(const boost::system::error_code& error,
								std::size_t byteSent);
		void startWrite(std::shared_ptr<Datagram> datagram,
						std::function<AfterWrite> processWrite,
						std::function<ErrorWrite> processError = {});

	private:
		udp::socket m_socket;
	};

	class Manager
	{
	public:
		explicit Manager();
		virtual ~Manager();
		auto& context() { return m_context; }

	private:
		boost::asio::io_context m_context;
		std::thread m_ioThread;
	};
}
