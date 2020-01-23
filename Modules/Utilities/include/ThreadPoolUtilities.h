#pragma once

#include <queue>
#include <map>
#include <set>

#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/optional.hpp>

#include <MitkUtilitiesExports.h>

namespace Utilities
{
  enum class TaskPriority
  {
    LOWEST,
    LOW,
    NORMAL,
    HI,
    CRITICAL
  };

  typedef std::function<void()> Task;

  class MITKUTILITIES_EXPORT ThreadPool : private boost::noncopyable
  {
  public:
    ThreadPool();
    explicit ThreadPool(size_t count);

    ~ThreadPool();

    void Stop();
    void Reset(size_t threadsCount=0);

    void AddThreads(size_t count);

    size_t Enqueue(const Task& task, TaskPriority priority = TaskPriority::NORMAL);

    bool Dequeue(size_t taskId);
    size_t Dequeue(const std::set<size_t>& taskIds);
    size_t DequeueAll();

    bool Empty() const;

    size_t WaitFirst(const std::set<size_t>& ids, volatile bool * stop = nullptr);
    void WaitAll(std::set<size_t> ids, volatile bool * stop = nullptr);
    void WaitAll(volatile bool * stop = nullptr);

    bool Check(const std::set<size_t>& ids) const;

    static ThreadPool& Instance();

  private:
    typedef boost::unique_lock<boost::shared_mutex> UniqueLock;
    typedef boost::shared_lock<boost::shared_mutex> SharedLock;

    void AddThreadsImpl(size_t count);

    size_t DoTask();
    template <typename TCheck>
    void Wait(const TCheck& check);

    boost::asio::io_service m_service;
    boost::optional<boost::asio::io_service::work> m_work;
    mutable boost::shared_mutex m_guard;
    size_t m_init_size;
    std::unique_ptr<boost::thread_group> m_pool;
    boost::system::error_code m_error;

    boost::shared_mutex m_queueGuard;
    size_t m_id;
    std::priority_queue<std::pair<TaskPriority, size_t>> m_queue;

    mutable boost::shared_mutex m_taskGuard;
    std::map<size_t, Task> m_task;
    std::set<size_t> m_runing;
    volatile size_t m_count;

    boost::shared_mutex m_eventGuard;
    boost::condition_variable_any m_event;
  };

  class MITKUTILITIES_EXPORT TaskGroup : private boost::noncopyable
  {
  public:
    TaskGroup();
    explicit TaskGroup(ThreadPool& pool);

    ~TaskGroup();

    void Enqueue(const Task& task, TaskPriority priority = TaskPriority::NORMAL);

    bool WaitFirst(volatile bool * stop = nullptr);
    bool WaitAll(volatile bool * stop = nullptr);

    void Stop();

    bool Empty(bool check = false) const;

  private:
    typedef boost::recursive_mutex::scoped_lock Lock;

    ThreadPool& m_pool;

    mutable boost::recursive_mutex m_mutex;
    std::set<size_t> m_ids;
  };

  class MITKUTILITIES_EXPORT NoLockedTask : private boost::noncopyable
  {
  public:
    NoLockedTask(ThreadPool& pool, const Task& task, TaskPriority priority = TaskPriority::NORMAL);
    explicit NoLockedTask(const Task& task, TaskPriority priority = TaskPriority::NORMAL);

    ~NoLockedTask();

    void Wait();

    void Stop();

    bool Complete(bool check = true) const;

  private:
    TaskGroup m_group;
  };
}