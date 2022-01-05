#pragma once
#include <boost/asio.hpp>

namespace Net
{

inline std::vector<char> makeData(std::string_view buffer)
{
    return std::vector<char>(buffer.data(), buffer.data() + buffer.size());
}

inline std::string_view toView(const std::vector<char>& buffer)
{
    return std::string_view(buffer.data(), buffer.size());
}

class Manager
{
public:
    explicit Manager();
    virtual ~Manager();

    inline auto& context() { return m_context; }
    inline auto& context() const { return m_context; }

protected:
    virtual void ioErrorOccured(const boost::system::error_code& error);

private:
    boost::asio::io_context m_context;
    std::thread m_ioThread;
};

}
