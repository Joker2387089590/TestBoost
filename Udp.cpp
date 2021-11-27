#include "Udp.h"

namespace Udp {

using boost::system::error_code;

Node::Node(boost::asio::io_context& context) : m_socket(context) {}

bool Node::bind(const udp::endpoint& endpoint)
{
	error_code ec;
	m_socket.bind(endpoint, ec);
	return !ec;
}

void Node::startRead(std::function<AfterReceive> processReceive,
					 std::function<ErrorReceive> processError)
{
	if(!processError) processError = [](auto&&...) { return true; };

	std::tuple args{std::make_shared<Datagram>(), std::move(processReceive), std::move(processError)};
	using Args = decltype(args);

	auto recurse = [self = shared_from_this()](auto recurse, Args args) -> void
	{
		auto& [datagram, processWrite, processError] = args;
		auto& endpoint = datagram->endpoint();
		auto handle = [self, recurse, args = std::move(args)]
					  (const error_code& error, std::size_t)
		{
			auto& [datagram, processReceive, processError] = args;
			auto& [endpoint, buffer] = *datagram;
			try
			{
				if(error) throw error;

				udp::socket::bytes_readable readable(true);
				self->m_socket.io_control(readable);

				auto bytesTransferred = readable.get();
				buffer.resize(bytesTransferred);

				auto asioBuf = boost::asio::buffer(buffer);
				self->m_socket.receive_from(std::move(asioBuf), endpoint, 0);
			}
			catch (const error_code& error)
			{
				if(!processError(*datagram, error)) return;
			}
			if(!processReceive(*datagram)) return;

			*datagram = {};
			recurse(recurse, std::move(args));
		};

		using boost::asio::null_buffers;
		self->m_socket.async_receive_from(null_buffers{}, endpoint, std::move(handle));
	};
	recurse(recurse, std::move(args));
}

void Node::startWrite(std::shared_ptr<Datagram> datagram,
					  std::function<AfterWrite> processWrite,
					  std::function<ErrorWrite> processError)
{
	if(!datagram) throw std::exception();
	if(!processError) processError = [](auto&&...) { return true; };

	std::tuple args{datagram, std::move(processWrite), std::move(processError)};
	using Args = decltype(args);

	auto recurse = [self = shared_from_this()](auto recurse, Args args) -> void
	{
		auto& [datagram, processWrite, processError] = args;
		auto asioBuf = boost::asio::buffer(std::as_const(datagram->data()));
		auto handle = [self, recurse, args = std::move(args)]
					  (error_code error, std::size_t byteTransfered)
		{
			auto& [datagram, processWrite, processError] = args;
			bool goingOn = !error ? processWrite() : processError(error, byteTransfered);
			if(goingOn) recurse(recurse, std::move(args));
		};
		self->m_socket.async_send_to(std::move(asioBuf), datagram->endpoint(), std::move(handle));
	};
	recurse(recurse, std::move(args));
}

Manager::Manager() : m_context{}
{
	std::atomic_bool wait = false;
	m_ioThread = std::thread([this, &wait]()
	{
		boost::asio::executor_work_guard work{m_context.get_executor()};
		wait = true;
		m_context.run();
	});
	while(!wait) {}
}

Manager::~Manager()
{
	m_context.stop();
	m_ioThread.join();
}

}
