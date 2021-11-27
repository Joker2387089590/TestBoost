#include "Udp.h"

int main()
{
	using namespace boost::asio;
	io_context io;
	io.run();
	return 0;
}
