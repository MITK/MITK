#pragma once

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/optional.hpp>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  class MITKUTILITIES_EXPORT ThreadPool : private boost::noncopyable
  {
  public:
    ThreadPool();
    ~ThreadPool();

    template <typename TTask>
    void Enqueue(const TTask& task);

    static ThreadPool& Instance();

  private:
    boost::asio::io_service m_service;
    boost::optional<boost::asio::io_service::work> m_work;
    boost::thread_group m_pool;
    boost::system::error_code m_error;
  };

  template <typename TTask>
  void ThreadPool::Enqueue(const TTask& task)
  {
    m_service.post(task);
  }

  class MITKUTILITIES_EXPORT TaskGroup : private boost::noncopyable
  {
  public:
    TaskGroup();
    explicit TaskGroup(ThreadPool& pool);

    ~TaskGroup();

    template <typename TTask>
    bool Enqueue(const TTask& task);

    bool WaitFirst();
    bool WaitAll();

    void Stop();
    bool Empty() const;

  private:
    typedef boost::recursive_mutex::scoped_lock Lock;

    bool Wait(Lock& lock, bool isMain);

    ThreadPool& m_pool;
    boost::recursive_mutex m_mutex;
    boost::condition_variable_any m_event;
    volatile size_t m_count;
    volatile bool m_valid;
  };

  template <typename TTask>
  bool TaskGroup::Enqueue(const TTask& task)
  {
    {
      const Lock lock(m_mutex);
      if (!m_valid) {
        return false;
      }
      ++m_count;
    }
    m_pool.Enqueue([this, task] {
      if (!m_valid) {
        return;
      }
      task();
      const Lock lock(m_mutex);
      --m_count;
      m_event.notify_all();
    });
    return true;
  }
}