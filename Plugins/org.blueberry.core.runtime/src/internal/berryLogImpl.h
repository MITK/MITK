/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  ~LogImpl() override;

  void AddLogListener(ILogListener* listener) override;

  QSharedPointer<ctkPlugin> GetBundle() const override;

  void Log(const SmartPointer<IStatus>& status) override;

  void RemoveLogListener(ILogListener* listener) override;
};

}

#endif // BERRYLOGIMPL_H
