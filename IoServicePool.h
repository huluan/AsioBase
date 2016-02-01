#ifndef EMNET_IO_SERVICE_POOL_H__
#define EMNET_IO_SERVICE_POOL_H__
#pragma once

#include "boost/asio.hpp"
#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"

namespace EMNet
{
    class IoServicePool : private boost::noncopyable
    {
    public:
        explicit IoServicePool(std::size_t poolSize);

        void Run();
        void Stop();
        boost::asio::io_service & GetIoService();

    private:
        typedef boost::shared_ptr<boost::asio::io_service>		IoServicePtr;
        typedef boost::shared_ptr<boost::asio::io_service::work> WorkPtr;

        std::vector<IoServicePtr> m_IoService;
        std::vector<WorkPtr> m_Work;
        std::size_t m_NextIoService;
    };
}
#endif
