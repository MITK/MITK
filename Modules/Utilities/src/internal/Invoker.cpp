#include "Invoker.h"

Q_DECLARE_METATYPE(Utilities::ExecuteProc);

namespace Utilities
{
  Invoker::Invoker()
  {
    qRegisterMetaType<Utilities::ExecuteProc>("Utilities::ExecuteProc");
  }

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