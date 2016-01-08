#include "IoServicePool.hpp"
#include "boost/thread.hpp"
#include "boost/bind.hpp"

namespace EMNet
{
	IoServicePool::IoServicePool(std::size_t poolSize) : m_NextIoService(0)
	{
		if (poolSize == 0)
		{
			throw std::runtime_error("IoServicePoll size is 0");
		}

		for (std::size_t i = 0; i < poolSize; ++i)
		{
			IoServicePtr ioService(new boost::asio::io_service);
			WorkPtr work(new boost::asio::io_service::work(*ioService));
			m_IoService.push_back(ioService);
			m_Work.push_back(work);
		}
	}

	void IoServicePool::Run()
	{
		std::vector<boost::shared_ptr<boost::thread>> threads;
		
		for (auto val : m_IoService)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&boost::asio::io_service::run, val)));
			threads.push_back(thread);
		}

		for (auto th : threads)
		{
			th->join();
		}
	}

	void IoServicePool::Stop()
	{
		for (auto io : m_IoService)
			io->stop();
	}

	boost::asio::io_service &IoServicePool::GetIoService()
	{
		boost::asio::io_service &ioService = *m_IoService[m_NextIoService];
		++m_NextIoService;
		if (m_NextIoService == m_IoService.size())
			m_NextIoService = 0;
		return ioService;
	}
}