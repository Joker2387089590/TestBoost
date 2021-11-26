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

	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		Node(boost::asio::io_context& context) : m_socket(context)
		{

		}

		using AfterReceive = bool(Datagram& datagram);
		using ErrorReceive = bool(Datagram& incompleteDatagram,
								  const boost::system::error_code& error);
		void startRead(std::function<AfterReceive> processReceive,
					   std::function<ErrorReceive> processError = {})
		{
			if(!processError) processError = [](auto&&...) { return true; };
			auto datagram = std::make_shared<Datagram>(udp::endpoint{}, emptyBuffer());
			fakeRecursive(std::move(processReceive), std::move(processError), std::move(datagram));
		}

	private:
		static std::vector<char> emptyBuffer(std::size_t size = 1024)
		{
			return std::vector<char>(size, char(0));
		}

		void fakeRecursive(std::function<AfterReceive> processReceive,
						   std::function<ErrorReceive> processError,
						   std::shared_ptr<Datagram> datagram)
		{
			auto handle = [self = shared_from_this(),
						   processReceive = std::move(processReceive),
						   processError = std::move(processError),
						   datagram]
			(const boost::system::error_code& error, std::size_t )
			{
				if(error)
				{
					processError(*datagram, error);
					return;
				}

				udp::socket::bytes_readable readable(true);
				boost::system::error_code errorIoControl;
				self->m_socket.io_control(readable, errorIoControl);
				self->m_socket.io_control(readable);
				if(error)
				{
					processError(*datagram, error);
					return;
				}

				auto& [endpoint, buffer] = *datagram; []{}();
				bool goingOn = false;
				if(!error)
				{
					auto bytesTransferred = readable.get();
					buffer.resize(bytesTransferred);
					self->m_socket.receive_from(boost::asio::buffer(buffer.data(),
																	buffer.size()),
												endpoint,
												0, errorIoControl);
				}

				if(goingOn)
				{
					endpoint = {};
					buffer = emptyBuffer();
					self->fakeRecursive(std::move(processReceive),
										std::move(processError),
										std::move(datagram));
				}
			};

			auto& [endpoint, buffer] = *datagram;
			m_socket.async_receive_from(boost::asio::null_buffers{}, endpoint, std::move(handle));
		}

	public:
		using AfterWrite = bool();
		void startWrite(std::vector<char> message, std::function<AfterWrite> processWrite)
		{
			// m_socket.asy
		}
	private:
		udp::socket m_socket;
	};

	class Manager
	{
	public:
		explicit Manager() : m_context{}
		{
			std::atomic_bool wait = false;
			m_ioThread = std::thread([this, &wait]()
			{
				boost::asio::executor_work_guard work{m_context.get_executor()};
				wait = true;
				m_context.run();
			});
			while (!wait) {}
		}

		virtual ~Manager()
		{
			m_context.stop();
			m_ioThread.join();
		}

		auto& context() { return m_context; }

	private:
		boost::asio::io_context m_context;
		std::thread m_ioThread;
	};
}

int main()
{
	using namespace boost::asio;
	io_context io;
	io.run();
	return 0;
}
