/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryMainApplicationLauncher.h"

#include "berryApplicationContainer.h"

#include <service/application/ctkApplicationException.h>

#include <QMutexLocker>

namespace berry {

MainApplicationLauncher::MainApplicationLauncher(ApplicationContainer* appContainer)
  : appContainer(appContainer)
{
}

QVariant MainApplicationLauncher::run(const QVariant& context)
{
  appContainer->StartDefaultApp(false);
  ctkApplicationRunnable* mainHandle = GetMainHandle();
  if (mainHandle != nullptr)
  {
    return mainHandle->run(context);
  }
  throw ctkApplicationException(ctkApplicationException::APPLICATION_INTERNAL_ERROR,
                                "No application id has been found.");
}

void MainApplicationLauncher::stop()
{
  // force the application to quit
  ctkApplicationRunnable* handle = GetMainHandle();
  if (handle != nullptr)
  {
    handle->stop();
  }
}

void MainApplicationLauncher::Launch(ctkApplicationRunnable* app)
{
  QMutexLocker l(&mutex);
  launchMainApp = app;
}

ctkApplicationRunnable*MainApplicationLauncher::GetMainHandle() const
{
  QMutexLocker l(&mutex);
  return launchMainApp;
}

}
