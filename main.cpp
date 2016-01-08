#include <iostream>
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "ConnManager.hpp"
#include <vector>
#include "boost/signal.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <fstream>

boost::mutex gMutex;
std::ofstream gFile("test.txt");
boost::asio::io_service gIOSerivce;

void OnConn(EMNet::TcpConnSharedPtr ptr)
{
	ptr->Recv(10);
}

void OnRead(EMNet::TcpConnSharedPtr ptr, const boost::system::error_code &err,
	unsigned char *buff, std::size_t buffSize)
{
	if (!err)
	{
        {
            boost::lock_guard<boost::mutex> lock(gMutex);
            std::cout << buff << std::endl;
        }
        gFile << buff << "\n";
		ptr->Send(buff, buffSize);
	}
	else
		ptr->Close();
}

void OnWrite(EMNet::TcpConnSharedPtr ptr, const boost::system::error_code &err,
	std::size_t sendSize)
{
	if (!err)
	{
		ptr->Recv(10);
	}
	else
		ptr->Close();
}

#include "boost/progress.hpp"
#include "boost/thread.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

std::vector<unsigned short> vecPort = { 1860 };
EMNet::ConnManager testServer(vecPort, 25, OnConn, OnRead, OnWrite, 4096);

void SingalHandler(const boost::system::error_code &err, int signal)
{ 
    std::cout << "Server Stop" << std::endl;
    testServer.Stop();
}

using namespace boost;
int main()
{
    boost::asio::signal_set sig(gIOSerivce, SIGINT, SIGTERM);
    sig.async_wait(SingalHandler);
    gIOSerivce.poll_one();
    std::cout << "go"<< std::endl;
	testServer.Run();
    gFile.close();
    return 0;
}
