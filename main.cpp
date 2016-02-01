#include <iostream>
#include "boost/asio.hpp"
#include "boost/bind.hpp"
#include "ConnManager.h"
#include <vector>
#include "boost/signals2.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <fstream>
//#include "vld.h"

boost::mutex gMutex;
//std::ofstream gFile("test.txt");
boost::asio::io_service gIOSerivce;
int gIDSeed = 0;
size_t packSize = 1024;

extern EMNet::ConnManager testServer;

using EMNet::TcpConnSharedPtr;

//void OnConn(EMNet::ConnID id)
//{
//    TcpConnSharedPtr ptr = testServer.GetConn(id);
//    if (!ptr)
//    {
//        return;
//    }
//    ptr->Recv(packSize);
//}

//void OnRead(EMNet::ConnID id, const boost::system::error_code &err,
//	unsigned char *buff, std::size_t buffSize)
//{
//    TcpConnSharedPtr ptr = testServer.GetConn(id);
//    //std::cout << "OnRead: " << ptr.use_count() << std::endl;
//	if (!err)
//	{
//        /*
//        {
//            boost::lock_guard<boost::mutex> lock(gMutex);
//            std::cout << buff << std::endl;
//        }
//        gFile << buff << "\n";
//        */
//		ptr->Send(buff, buffSize);
//	}
//	else
//		ptr->Close();
//}

//void OnWrite(EMNet::ConnID id, const boost::system::error_code &err,
//	std::size_t sendSize)
//{
//    TcpConnSharedPtr ptr = testServer.GetConn(id);
//	if (!err)
//	{
//		ptr->Recv(packSize);
//	}
//	else
//		ptr->Close();
//}

#include "boost/progress.hpp"
#include "boost/thread.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

// 新的连接事件
void OnConn(EMNet::TcpConnSharedPtr ptr)
{
	ptr->Recv(packSize);
}

// 有读事件
void OnRead(EMNet::TcpConnSharedPtr ptr, const boost::system::error_code &err,
	unsigned char *buff, std::size_t buffSize)
{
	if (!err)
	{
		ptr->Send(buff, buffSize);
	}
	else
		ptr->Close();
}

// 有写完成事件
void OnWrite(EMNet::TcpConnSharedPtr ptr, const boost::system::error_code &err,
	std::size_t sendSize)
{
	if (!err)
	{
		ptr->Recv(packSize);
	}
	else
		ptr->Close();
}

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
    std::cout << "go" << std::endl;
    std::cin >> packSize;
    std::cout << "cout: " << packSize << std::endl;
	testServer.Run();
    //gFile.close();
    return 0;
}
