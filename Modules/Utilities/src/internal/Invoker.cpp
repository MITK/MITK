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
    if (!isGuiThread()) {
      QMetaObject::invokeMethod(this, "ExecProc", connType, Q_ARG(ExecuteProc, proc));
    } else {
      proc();
    }
  }

  void Invoker::ExecProc(const ExecuteProc& proc) const
  {
    proc();
  }
}