#include "Invoker.h"

namespace Utilities
{
  Q_DECLARE_METATYPE(ExecuteProc);

  void Invoker::ExecInMainThread(const ExecuteProc& proc, Qt::ConnectionType connType)
  {
    if (isGuiThread()) {
      proc();
    } else {
      QMetaObject::invokeMethod(this, "ExecProc", connType, Q_ARG(ExecuteProc, proc));
    }
  }

  void Invoker::ExecProc(const ExecuteProc& proc) const
  {
    proc();
  }
}