#include "TcpConnectBase.hpp"
#include "boost/bind.hpp"
#include "boost/asio.hpp"
#include "boost/make_shared.hpp"


namespace EMNet
{
	TcpConnectBase::TcpConnectBase(const SocketPtr &socket,
		const std::size_t buffSize,
		const ConnectCallback &connCb,
		const DataRecvCallback &recvCb,
		const DataSendCallback &sendCb) : m_Socket(socket),
		m_RecvBuffSize(buffSize),
		m_RecvBuff(boost::make_shared<std::vector<unsigned char>>(buffSize, 0)),
		m_ConnCallback(connCb),
		m_DataRecvCallback(recvCb),
		m_DataSendCallback(sendCb) { }

	void TcpConnectBase::Recv(std::size_t dataLen)
	{
		if (dataLen > m_RecvBuff->size())
		{
			throw std::runtime_error("DataLen is too long");
		}
		boost::asio::async_read(*m_Socket, boost::asio::buffer(m_RecvBuff->data(), dataLen),
			boost::bind(&TcpConnectBase::RecvHandler, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void TcpConnectBase::RecvHandler(const boost::system::error_code & error, std::size_t byteRecv)
	{
		m_DataRecvCallback(shared_from_this(), error, m_RecvBuff->data(), byteRecv);
	}

	void TcpConnectBase::Send(const unsigned char *data, std::size_t dataLen)
	{
		boost::lock_guard<boost::mutex> guard(m_ConnMutex);
		m_SendQueue.push_back(VecPacketPtr(boost::make_shared<std::vector<unsigned char>>(data, data + dataLen)));
		// ±£´æÓÐÐò·¢ËÍ
		if (1 == m_SendQueue.size())
		{
			auto packet = m_SendQueue.front();
			boost::asio::async_write(*m_Socket, boost::asio::buffer(packet->data(), packet->size()),
				boost::bind(&TcpConnectBase::SendHandler, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
	}

    void TcpConnectBase::Send(VecPacketPtr packetPtr)
    {
		boost::lock_guard<boost::mutex> guard(m_ConnMutex);
		m_SendQueue.push_back(packetPtr);
		if (1 == m_SendQueue.size())
		{
			auto packet = m_SendQueue.front();
			boost::asio::async_write(*m_Socket, boost::asio::buffer(packet->data(), packet->size()),
				boost::bind(&TcpConnectBase::SendHandler, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}

    }

	void TcpConnectBase::SendHandler(const boost::system::error_code &error, std::size_t byteSend)
	{
		if (!error)
		{
			boost::lock_guard<boost::mutex> guard(m_ConnMutex);
			m_SendQueue.pop_front();
			if (!m_SendQueue.empty())
			{
				auto nextPacket = m_SendQueue.front();
				boost::asio::async_write(*m_Socket, boost::asio::buffer(nextPacket->data(), nextPacket->size()),
					boost::bind(&TcpConnectBase::SendHandler, shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred));
			}
		}
		m_DataSendCallback(shared_from_this(), error, byteSend);
	}

	void TcpConnectBase::Close()
	{
        boost::lock_guard<boost::mutex> guard(m_ConnMutex);
		m_Socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		m_Socket->close();
	}
}