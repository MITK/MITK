#include "ThreadPoolUtilities.h"

#include <memory>

#include <boost/bind.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <QCoreApplication>

#include "ThreadUtilities.h"

namespace
{
  boost::shared_mutex s_guard;
  std::unique_ptr<Utilities::ThreadPool> s_instance;
}

namespace Utilities
{
  ThreadPool::ThreadPool()
    : m_work(boost::in_place<boost::asio::io_service::work>(boost::ref(m_service)))
    , m_id(0)
    , m_count(0)
    , m_init_size(boost::thread::hardware_concurrency())
  {
  }

  ThreadPool::ThreadPool(size_t count)
    : m_work(boost::in_place<boost::asio::io_service::work>(boost::ref(m_service)))
    , m_id(0)
    , m_count(0)
    , m_init_size(count)
  {
  }

  ThreadPool::~ThreadPool()
  {
    Stop();
  }

  ThreadPool& ThreadPool::Instance()
  {
    boost::upgrade_lock<boost::shared_mutex> lock(s_guard);
    if (!s_instance) {
      const boost::upgrade_to_unique_lock<boost::shared_mutex> guard(lock);
      if (!s_instance) {
        s_instance.reset(new ThreadPool());
      }
    }
    return *s_instance;
  }

  void ThreadPool::Stop()
  {
    {
      const boost::unique_lock<boost::shared_mutex> lock(m_guard);
      if (!m_work || m_init_size) {
        return;
      }
      m_work = boost::none;
    }
    m_pool.join_all();
  }

  void ThreadPool::Reset()
  {
    {
      const boost::unique_lock<boost::shared_mutex> lock(m_guard);
      if (m_work) {
        m_work = boost::none;
      }
      m_service.stop();
    }
    m_pool.join_all();

    const boost::unique_lock<boost::shared_mutex> lock(m_guard);
    m_service.reset();
    m_work = boost::in_place<boost::asio::io_service::work>(boost::ref(m_service));
  }

  void ThreadPool::AddThreadsImpl(size_t count)
  {
    for (size_t i = 0; i < count; ++i) {
      m_pool.create_thread(boost::bind(&boost::asio::io_service::run, &m_service, boost::ref(m_error)));
    }
  }

  void ThreadPool::AddThreads(size_t count)
  {
    const boost::unique_lock<boost::shared_mutex> lock(m_guard);

    if (m_init_size) {
      m_init_size += count;
      return;
    }

    AddThreadsImpl(count);
  }

  size_t ThreadPool::Enqueue(const Task& task, TaskPriority priority)
  {
    {
      const boost::unique_lock<boost::shared_mutex> lock(m_guard);
      if (!m_work) {
        return 0;
      }
      if (m_init_size) {
        AddThreadsImpl(m_init_size);
        m_init_size = 0;
      }
    }

    UniqueLock lockTask(m_taskGuard);
    const auto taskId = ++m_id;
    m_task.emplace(taskId, task);
    lockTask.unlock();

    UniqueLock lockQueue(m_queueGuard);
    m_queue.emplace(priority, taskId);
    lockQueue.unlock();

    m_service.post([this] { DoTask(); });
    return taskId;
  }

  bool ThreadPool::Dequeue(size_t taskId)
  {
    const UniqueLock lock(m_taskGuard);
    if (m_runing.end() != m_runing.find(taskId)) {
      return false;
    }
    m_task.erase(taskId);
    ++m_count;

    const SharedLock guard(m_eventGuard);
    m_event.notify_all();
    return true;
  }

  size_t ThreadPool::Dequeue(const std::set<size_t>& taskIds)
  {
    size_t n = taskIds.size();
    const UniqueLock lock(m_taskGuard);
    for (auto id : taskIds) {
      if (m_runing.end() == m_runing.find(id)) {
        m_task.erase(id);
        ++m_count;
      } else {
        --n;
      }
    }

    const SharedLock guard(m_eventGuard);
    m_event.notify_all();
    return n;
  }

  size_t ThreadPool::DequeueAll()
  {
    const UniqueLock lock(m_taskGuard);
    for (auto it = m_task.begin(); it != m_task.end(); ) {
      if (m_runing.end() == m_runing.find(it->first)) {
        it = m_task.erase(it);
        ++m_count;
      } else {
        ++it;
      }
    }

    const SharedLock guard(m_eventGuard);
    m_event.notify_all();
    return m_task.size();
  }

  bool ThreadPool::Empty() const
  {
    const SharedLock lock(m_taskGuard);
    return m_task.empty();
  }

  size_t ThreadPool::DoTask()
  {
    UniqueLock lockQueue(m_queueGuard);
    if (m_queue.empty()) {
      return 0;
    }
    auto info = m_queue.top();
    m_queue.pop();
    lockQueue.unlock();

    UniqueLock lockTask(m_taskGuard);
    auto it = m_task.find(info.second);
    if (m_task.end() == it) {
      return info.second;
    }
    m_runing.insert(it->first);
    lockTask.unlock();

    it->second();

    lockTask.lock();
    m_runing.erase(it->first);
    m_task.erase(it);
    ++m_count;

    const SharedLock guard(m_eventGuard);
    m_event.notify_all();
    return info.second;
  }

  template <typename TCheck>
  void ThreadPool::Wait(const TCheck& check)
  {
    boost::shared_lock<boost::shared_mutex> lock(m_guard);
    const bool isInPool = m_pool.is_this_thread_in();
    lock.unlock();

    if (isInPool) {
      while (!check()) {
        DoTask();
      }
      return;
    }

    if (isGuiThread()) {
      for (auto count = m_count; !check(); count = m_count) {
        while (count == m_count) {
          QCoreApplication::processEvents();
        }
      }
    } else {
      SharedLock lock(m_eventGuard);
      m_event.wait(lock, check);
    }
  }

  size_t ThreadPool::WaitFirst(const std::set<size_t>& ids, volatile bool * stop)
  {
    size_t id = 0;
    if (ids.empty()) {
      return id;
    }
    auto check = [this, &id, &ids, stop]() -> bool {
      if (stop && *stop) {
        return true;
      }
      const SharedLock lock(m_taskGuard);
      for (auto task : ids) {
        if (m_task.end() != m_task.find(task)) {
          continue;
        }
        id = task;
        return true;
      }
      return false;
    };
    Wait(check);
    return id;
  }

  void ThreadPool::WaitAll(std::set<size_t> ids, volatile bool * stop)
  {
    auto check = [this, &ids, stop]() -> bool {
      if (stop && *stop) {
        return true;
      }
      const SharedLock lock(m_taskGuard);
      for (auto it = ids.begin(); it != ids.end(); ) {
        if (m_task.end() != m_task.find(*it)) {
          ++it;
        } else {
          it = ids.erase(it);
        }
      }
      return ids.empty();
    };
    Wait(check);
  }

  void ThreadPool::WaitAll(volatile bool * stop)
  {
    auto check = [this, stop]() -> bool {
      if (stop && *stop) {
        return true;
      }
      const SharedLock lock(m_taskGuard);
      return m_task.empty();
    };
    Wait(check);
  }

  bool ThreadPool::Check(const std::set<size_t>& ids) const
  {
    const SharedLock lock(m_taskGuard);
    for (auto task : ids) {
      if (m_task.end() != m_task.find(task)) {
        return false;
      }
    }
    return true;
  }

  TaskGroup::TaskGroup()
    : m_pool(ThreadPool::Instance())
  {
  }

  TaskGroup::TaskGroup(ThreadPool& pool)
    : m_pool(pool)
  {
  }

  TaskGroup::~TaskGroup()
  {
    Stop();
  }

  void TaskGroup::Stop()
  {
    Lock lock(m_mutex);
    const auto ids = m_ids;
    lock.unlock();
    m_pool.Dequeue(std::move(ids));
  }

  bool TaskGroup::WaitFirst(volatile bool * stop)
  {
    if (stop && *stop) {
      return false;
    }

    Lock lock(m_mutex);
    const auto ids = m_ids;
    lock.unlock();

    const auto id = m_pool.WaitFirst(std::move(ids), stop);
    if (stop && *stop) {
      return false;
    }

    lock.lock();
    m_ids.erase(id);
    return !stop || !*stop;
  }

  bool TaskGroup::WaitAll(volatile bool * stop)
  {
    while (!stop || !*stop) {
      Lock lock(m_mutex);
      const auto ids = m_ids;
      lock.unlock();

      m_pool.WaitAll(ids, stop);
      if (stop && *stop) {
        return false;
      }

      lock.lock();
      for (auto id : ids) {
        m_ids.erase(id);
      }
      if (m_ids.empty()) {
        break;
      }
    }
    return !stop || !*stop;
  }

  bool TaskGroup::Empty(bool check) const
  {
    const Lock lock(m_mutex);
    return check ? m_pool.Check(m_ids) : m_ids.empty();
  }

  void TaskGroup::Enqueue(const Task& task, TaskPriority priority)
  {
    const Lock lock(m_mutex);
    m_ids.insert(m_pool.Enqueue(task, priority));
  }

  NoLockedTask::NoLockedTask(ThreadPool& pool, const Task& task, TaskPriority priority)
    : m_group(pool)
  {
    m_group.Enqueue(task, priority);
  }

  NoLockedTask::NoLockedTask(const Task& task, TaskPriority priority)
  {
    m_group.Enqueue(task, priority);
  }

  NoLockedTask::~NoLockedTask()
  {
    m_group.WaitAll();
  }

  void NoLockedTask::Wait()
  {
    m_group.WaitAll();
  }

  void NoLockedTask::Stop()
  {
    m_group.Stop();
  }

  bool NoLockedTask::Complete(bool check) const
  {
    return m_group.Empty(check);
  }
}
