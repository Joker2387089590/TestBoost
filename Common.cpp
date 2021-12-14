#include <Common.h>

namespace Net
{
using boost::system::error_code;

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
