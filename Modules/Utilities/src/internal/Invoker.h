#pragma once

#include <QMetaMethod>

#include "ThreadUtilities.h"

namespace Utilities
{
  class Invoker : public QObject
  {
    Q_OBJECT

  public:
    Invoker();

    void ExecInMainThread(const ExecuteProc& proc, Qt::ConnectionType connType);

  private:
    Q_INVOKABLE void ExecProc(const ExecuteProc& proc) const;
  };
}