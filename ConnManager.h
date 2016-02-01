#ifndef EMNET_CONN_MANAGER_H__
#define EMNET_CONN_MANAGER_H__
#pragma once
#include "boost/asio.hpp"
#include "boost/noncopyable.hpp"
#include "boost/function.hpp"
#include "IoServicePool.h"
#include "TcpConnectBase.h"

#include <string>
#include <unordered_map>

namespace EMNet
{
    typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> AcceptorPtr;
    class ConnManager : private boost::noncopyable
    {
    public:
        explicit ConnManager(const std::vector<unsigned short> &ports,
            std::size_t ioServicePoolSize,
            const ConnectCallback &connCb,
            const DataRecvCallback &recvCb,
            const DataSendCallback &sendCb,
            const std::size_t recvBuffSize);

        void Run();
        void Stop();
        inline TcpConnSharedPtr GetConn(const ConnID id)
        {
            //TODO: Ëø
            auto gotConn = m_Clients.find(id);
            if (gotConn != m_Clients.end())
            {
                auto connWeakPtr = gotConn->second;
                if (!connWeakPtr.expired())
                {
                    return connWeakPtr.lock();
                }
                else
                {
                    m_Clients.erase(id);
                }
            }
            return TcpConnSharedPtr();
        }

    private:
        void HandleAccept(SocketPtr socket, AcceptorPtr acceptor, const boost::system::error_code &err);

    private:
        IoServicePool m_IoServicePool;
        typedef std::vector<AcceptorPtr> VecAcceptorPtr;
        VecAcceptorPtr m_Acceptors;
        ConnectCallback m_ConnCallback;
        DataRecvCallback m_DataRecvCallback;
        DataSendCallback m_DataSendCallback;
        std::vector<unsigned short> m_Ports;
        boost::shared_ptr<boost::thread>  m_AcceptThread;
        boost::asio::io_service m_AcceptIoService;
        boost::asio::io_service::work	m_AcceptIoServiceWork;
        std::size_t m_RecvBuffSize;
        ConnID m_SocketIDSeed;
        std::unordered_map<ConnID, TcpConnWeakPtr> m_Clients;
    };
}
#endif