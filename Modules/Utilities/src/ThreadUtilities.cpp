#include "ThreadUtilities.h"

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
    return QThread::currentThread() == QCoreApplication::instance()->thread();
  }

  void execInMainThreadAsync(const ExecuteProc& proc)
  {
    s_invoker.ExecInMainThread(proc, Qt::QueuedConnection);
  }

  void execInMainThreadSync(const ExecuteProc& proc)
  {
    s_invoker.ExecInMainThread(proc, Qt::BlockingQueuedConnection);
  }
}