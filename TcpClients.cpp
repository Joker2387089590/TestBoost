#include "TcpClients.h"

namespace
{
constexpr std::size_t bufferSize = 1024;
auto emptyBuffer() { return std::vector<char>(bufferSize, '\0'); }
}

namespace Net::Tcp::Client
{
Node::Node(boost::asio::io_context &context) :
    m_socket(context),
    m_buffer(emptyBuffer())
{}

std::size_t Node::write(std::string_view buffer)
{
    return m_socket.write_some(boost::asio::buffer(buffer.data(), buffer.size()));
}

std::vector<char> Node::read()
{
    std::vector<char> buf(1024, '\0');
    auto size = m_socket.read_some(boost::asio::buffer(buf));
    buf.resize(size);
    return buf;
}

bool Node::failRead([[maybe_unused]] std::vector<char> incompleteBuffer,
[[maybe_unused]] const boost::system::error_code& ec)
{
    if(ec == boost::asio::error::eof) return false;
    return true;
}

void Node::startAsyncRead()
{
    auto buf = boost::asio::buffer(m_buffer.data(), bufferSize);
    auto handle = [self = shared_from_this()](const boost::system::error_code& ec,
            std::size_t byteTransfered)
    {
        self->m_buffer.resize(byteTransfered);
        if(!ec)
            self->processRead(std::exchange(self->m_buffer, emptyBuffer()));
        else if(!self->failRead(std::exchange(self->m_buffer, emptyBuffer()), ec))
            return;
        self->startAsyncRead(); // 往 io_context 再添加一次异步读任务，因为是异步，所以没有递归
    };
    m_socket.async_read_some(buf, std::move(handle));
}

}

