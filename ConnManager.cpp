#include "ConnManager.hpp"
#include <sstream>
#include "boost/make_shared.hpp"

namespace EMNet
{
	ConnManager::ConnManager(const std::vector<unsigned short> &ports, std::size_t ioServicePoolSize,
		const ConnectCallback &connCb,
		const DataRecvCallback &recvCb,
		const DataSendCallback &sendCb,
		const std::size_t recvBuffSize) :
		m_ConnCallback(connCb),
		m_DataRecvCallback(recvCb),
		m_DataSendCallback(sendCb),
		m_IoServicePool(ioServicePoolSize),
		m_Ports(ports),
		m_AcceptIoServiceWork(m_AcceptIoService),
		m_RecvBuffSize(recvBuffSize)
    {

    }

    void ConnManager::Run()
    {
		m_AcceptThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &m_AcceptIoService)));
        std::vector<boost::asio::ip::tcp::endpoint> endpoints;
        for (auto val : m_Ports)
        {
            endpoints.push_back(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), val));
        }

		for (auto ep : endpoints)
		{
			AcceptorPtr acceptor(boost::make_shared<boost::asio::ip::tcp::acceptor>(m_AcceptIoService));
			try
			{
				acceptor->open(ep.protocol());
				acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
				acceptor->bind(ep);
				acceptor->listen();
			}
			catch (const std::exception &e)
			{
				std::stringstream ss;
				ss << "Start Server failed on port [" << ep.port() << "] \n" << e.what();
				throw std::runtime_error(ss.str());
			}
			m_Acceptors.push_back(acceptor);

			SocketPtr socket(boost::make_shared<boost::asio::ip::tcp::socket>(m_IoServicePool.GetIoService()));

			acceptor->async_accept(*socket, boost::bind(&ConnManager::HandleAccept, this,
				socket, acceptor, boost::asio::placeholders::error));
		}
		m_IoServicePool.Run();
    }

	void ConnManager::HandleAccept(SocketPtr socket, AcceptorPtr acceptor, const boost::system::error_code &err)
	{
		if (!err)
		{
			TcpConnSharedPtr newConn(boost::make_shared<TcpConnectBase>(socket, m_RecvBuffSize,
				m_ConnCallback, m_DataRecvCallback, m_DataSendCallback));
			newConn->Start();

		}
        SocketPtr newSocket(boost::make_shared<boost::asio::ip::tcp::socket>(m_IoServicePool.GetIoService()));

		acceptor->async_accept(*newSocket, boost::bind(&ConnManager::HandleAccept, this,
			newSocket, acceptor, boost::asio::placeholders::error));
	}

	void ConnManager::Stop()
	{
		for (auto val : m_Acceptors)
		{
			val->close();
		}
		m_IoServicePool.Stop();
	}
}