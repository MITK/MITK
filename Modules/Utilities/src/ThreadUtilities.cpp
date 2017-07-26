#include "ThreadUtilities.h"

#include <future>

#include <QMetaMethod>
#include <QThread>
#include <QCoreApplication>

#include "internal/Invoker.h"

namespace
{
  Utilities::Invoker s_invoker;
}

namespace Utilities
{
  bool isGuiThread()
  {
    auto core = QCoreApplication::instance();
    return core ? QThread::currentThread() == core->thread() : false;
  }

  void execInMainThreadAsync(const ExecuteProc& proc)
  {
    s_invoker.ExecInMainThread(proc, Qt::QueuedConnection);
  }

  void execInMainThreadSync(const ExecuteProc& proc)
  {
    s_invoker.ExecInMainThread(proc, Qt::BlockingQueuedConnection);
  }

  void execUnlocked(const ExecuteProc& proc)
  {
    auto future = std::async(std::launch::async, proc);
    if (isGuiThread()) {
      while (future.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
        QCoreApplication::processEvents();
      }
    } else {
      future.wait();
    }
  }
}