#ifndef EMNET_TCP_CONNECT_BASE_H__
#define EMNET_TCP_CONNECT_BASE_H__
#pragma once
#include "boost/enable_shared_from_this.hpp"
#include "boost/noncopyable.hpp"
#include "boost/function.hpp"
#include "boost/asio.hpp"
#include "boost/thread.hpp"

#include <deque>


namespace EMNet
{
    class TcpConnectBase;
    typedef boost::shared_ptr<TcpConnectBase> TcpConnSharedPtr;
    typedef boost::weak_ptr<TcpConnectBase> TcpConnWeakPtr;
    typedef std::size_t ConnID;
    typedef boost::function< void(TcpConnSharedPtr)> ConnectCallback;
    typedef boost::function< void(TcpConnSharedPtr, const boost::system::error_code &, unsigned char *, std::size_t)> DataRecvCallback;
    typedef boost::function< void(TcpConnSharedPtr, const boost::system::error_code &, std::size_t)> DataSendCallback;
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> SocketPtr;
    typedef boost::shared_ptr<std::vector<unsigned char>> VecPacketPtr;
    class TcpConnectBase : public boost::enable_shared_from_this<TcpConnectBase>,
        private boost::noncopyable
    {
    public:
        explicit TcpConnectBase(const SocketPtr &socket,
            const std::size_t buffSize,
            const ConnectCallback &connCb,
            const DataRecvCallback &recvCb,
            const DataSendCallback &sendCb,
            const ConnID socketID);

        inline void Start() { m_ConnCallback(shared_from_this()); }
        void Recv(std::size_t recvLen);
        void Send(const unsigned char *data, std::size_t dataLen);
        void Send(VecPacketPtr packetPtr);
        void Close();

        inline void RegisterConnectCallback(const ConnectCallback &callback) { m_ConnCallback = callback; }
        inline void RegisterDataRecvCallback(const DataRecvCallback &callback) { m_DataRecvCallback = callback; }
        inline void RegisterDataSendCallback(const DataSendCallback &callback) { m_DataSendCallback = callback; }
        inline std::size_t GetConnID() const { return m_ConnID; }
    private:
        void SendHandler(const boost::system::error_code &error, std::size_t bytesRead);
        void RecvHandler(const boost::system::error_code &error, std::size_t bytesRead);

    private:
        SocketPtr m_Socket;
        std::size_t m_RecvBuffSize;
        boost::shared_ptr<std::vector<unsigned char>> m_RecvBuff;
        std::deque<VecPacketPtr> m_SendQueue;
        ConnectCallback m_ConnCallback;
        DataRecvCallback m_DataRecvCallback;
        DataSendCallback m_DataSendCallback;
        boost::mutex m_ConnMutex;
        ConnID m_ConnID;
    };
}
#endif