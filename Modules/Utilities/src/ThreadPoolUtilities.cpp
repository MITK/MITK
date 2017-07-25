#include "ThreadPoolUtilities.h"

#include <boost/bind.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <QCoreApplication>

#include "ThreadUtilities.h"

namespace
{
  Utilities::ThreadPool s_instance;
}

namespace Utilities
{
  ThreadPool::ThreadPool()
    : m_work(boost::in_place<boost::asio::io_service::work>(boost::ref(m_service)))
  {
    const auto count = boost::thread::hardware_concurrency();
    for (size_t i = 0; i < count; ++i) {
      m_pool.create_thread(boost::bind(&boost::asio::io_service::run, &m_service, boost::ref(m_error)));
    }
  }

  ThreadPool::~ThreadPool()
  {
    m_work = boost::none;
    m_pool.join_all();
  }

  ThreadPool& ThreadPool::Instance()
  {
    return s_instance;
  }

  TaskGroup::TaskGroup()
    : m_pool(ThreadPool::Instance())
    , m_count(0)
    , m_valid(true)
  {
  }

  TaskGroup::TaskGroup(ThreadPool& pool)
    : m_pool(pool)
    , m_count(0)
    , m_valid(true)
  {
  }

  TaskGroup::~TaskGroup()
  {
    Stop();
  }

  bool TaskGroup::WaitFirst()
  {
    Lock lock(m_mutex);
    if (!m_count)
      return true;
    return Wait(lock, isGuiThread());
  }

  bool TaskGroup::WaitAll()
  {
    Lock lock(m_mutex);
    for (const auto isMain = isGuiThread(); !!m_count; Wait(lock, isMain)) {}
    return m_valid && !!m_count;
  }

  bool TaskGroup::Wait(Lock& lock, bool isMain)
  {
    if (isMain) {
      while (boost::cv_status::timeout == m_event.wait_for(lock, boost::chrono::milliseconds(10)))
      {
        QCoreApplication::processEvents();
      }
    } else {
      m_event.wait(lock);
    }
    return m_valid;
  }

  void TaskGroup::Stop()
  {
    Lock lock(m_mutex);
    m_valid = false;
    m_event.notify_all();
  }

  bool TaskGroup::Empty() const
  {
    return !m_count;
  }
}
