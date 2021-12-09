#include "Udp.h"

namespace Udp
{
using boost::system::error_code;

Node::Ptr Node::make(Manager& manager) { return Ptr(new Node(manager.context())); }

Node::Node(boost::asio::io_context& context) : m_socket(context) {}

bool Node::bind(const udp::endpoint& endpoint)
{
	error_code ec;
	m_socket.bind(endpoint, ec);
	return !ec;
}

void Node::startRead(std::function<AfterReceive> afterReceive,
					 std::function<ErrorReceive> errroReceive)
{
	// 生成内部使用的数据报，并打包数据报与函数
	std::tuple args{std::make_shared<Datagram>(), std::move(afterReceive), std::move(errroReceive)};
	using Args = decltype(args);

	// “递归” 地监听 receive 事件
	auto recurse = [self = shared_from_this()](auto recurse, Args args) -> void
	{
		// 在生成函数对象 `callback` 前，取得对 endpoint 对象的引用
		auto& [datagram, afterReceive, errorReceive] = args;
		auto& endpoint = datagram->endpoint();

		// 注册到 asio 的回调函数，捕获列表的 `std::move(args)` 会导致 args 变为空
		auto callback = [self, recurse, args = std::move(args)]
						(const error_code& error, std::size_t)
						mutable // 如果没有 mutable，则 `args` 是 const 的，`std::move(args)` 就无法移动
		{
			auto& [datagram, afterReceive, errorReceive] = args;
			auto& [endpoint, buffer] = *datagram;
			try
			{
				if(error) throw error;

				// 获取可以不阻塞地读取的所有数据的长度
				udp::socket::bytes_readable readable(true);
				self->m_socket.io_control(readable);
				auto bytesTransferred = readable.get();

				// 用 asio::upd::socket 的同步接口读取缓冲区的所有数据
				buffer.resize(bytesTransferred);
				self->m_socket.receive_from(boost::asio::buffer(buffer), endpoint, 0);
			}
			catch (const error_code& error)
			{
				// 如果没有设置错误处理，就默认继续
				if(errorReceive && !errorReceive(*datagram, error)) return;
			}
			if(afterReceive && !afterReceive(*datagram)) return;

			// 清空数据报，下一次会复用它
			endpoint = {}; buffer.clear();

			// `callback` 是被 io_context 调用的，运行到这里时，调用栈里其实已经没有 `recurse` 了
			recurse(recurse, std::move(args));
		};

		// 使用空的 buffer 来接收数据，则数据会被留在 socket 内部的缓冲区里
		using boost::asio::null_buffers;
		self->m_socket.async_receive_from(null_buffers{}, endpoint, std::move(callback));
	};

	// 开始向 upd::socket 添加异步监听
	recurse(recurse, std::move(args));
}

void Node::startWrite(std::shared_ptr<Datagram> datagram,
					  std::function<AfterWrite> afterWrite,
					  std::function<ErrorWrite> errorWrite)
{
	if(!datagram) throw std::exception();
	auto asioBuf = boost::asio::buffer(std::as_const(datagram->data()));
	auto& endpoint = datagram->endpoint();

	std::tuple args{std::move(datagram), std::move(afterWrite), std::move(errorWrite)};
	auto callback = [self = shared_from_this(), args = std::move(args)]
					(const error_code& error, std::size_t byteTransfered) mutable
	{
		auto& [datagram, processWrite, processError] = args; []{}();
		if(!error)
		{
			if(processWrite) processWrite();
		}
		else
		{
			if(processError) processError(error, byteTransfered);
		}
	};
	m_socket.async_send_to(std::move(asioBuf), endpoint, std::move(callback));
}

Manager::Manager() : m_context{}, m_ioThread{}
{
	std::promise<void> wait;
	m_ioThread = std::thread([this, &wait]()
	{
		boost::asio::executor_work_guard work{m_context.get_executor()};
		wait.set_value();
		try
		{
			m_context.run();
		}
		catch (const error_code& error)
		{
			ioErrorOccured(error);
		}
	});
	wait.get_future().get();
}

Manager::~Manager()
{
	m_context.stop();
	m_ioThread.join();
}

void Manager::ioErrorOccured([[maybe_unused]] const error_code& error)
{
	[[maybe_unused]] std::string msg = error.message();
}

}
