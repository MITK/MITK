/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYLOGIMPL_H
#define BERRYLOGIMPL_H

#include "berryILog.h"

#include <QSharedPointer>

namespace berry {

// This is preliminary implementation until we have full
// CTK logging service integration
class LogImpl : public ILog
{

private:

  QSharedPointer<ctkPlugin> plugin;

public:

  LogImpl(const QSharedPointer<ctkPlugin>& plugin);

  ~LogImpl();

  void AddLogListener(ILogListener* listener);

  QSharedPointer<ctkPlugin> GetBundle() const;

  void Log(const SmartPointer<IStatus>& status);

  void RemoveLogListener(ILogListener* listener);
};

}

#endif // BERRYLOGIMPL_H
