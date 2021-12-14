#pragma once
#include <thread>
#include <iostream>
#include <boost/asio.hpp>
#include <IntWrapper.h>

namespace Tcp
{
	class ManagerBase
	{
	public:
		explicit ManagerBase();
		virtual ~ManagerBase();
	protected:
		boost::asio::io_context m_context;
		std::thread m_ioThread;
	};
}

namespace Tcp::Client
{
	using boost::asio::ip::tcp;

	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		explicit Node(boost::asio::io_context& context);
		virtual ~Node() = default;

		std::size_t write(std::string_view buffer); // 同步写
		std::vector<char> read();					// 同步读

		void startAsyncRead();	// 开始监听异步读

		template<typename ClientNode,
				 std::enable_if_t<std::is_base_of_v<Node, ClientNode>, int>>
		friend std::shared_ptr<ClientNode> connectTo(boost::asio::io_context& context,
													 std::string_view addr, u16 port);

	protected:
		virtual void processRead(std::vector<char> buffer) = 0;

		// 返回 true 则继续运行
		virtual bool failRead(std::vector<char> incompleteBuffer,
							  const boost::system::error_code& ec);

	protected:
		tcp::socket m_socket;
		std::vector<char> m_buffer;
	};

	template<typename ClientNode,
			 std::enable_if_t<std::is_base_of_v<Node, ClientNode>, int> = 0>
	std::shared_ptr<ClientNode> connectTo(boost::asio::io_context& context,
										  std::string_view addr, u16 port)
	try
	{

		auto ip = boost::asio::ip::make_address(addr);
		auto endpoint = tcp::endpoint(ip, port);
		auto client = std::make_shared<ClientNode>(context);
		client->m_socket.connect(endpoint);
		return client;
	}
	catch(const boost::system::system_error& e)
	{
		std::cerr << e.what() << std::endl;
		return nullptr;
	}
}
