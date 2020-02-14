/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYMAINAPPLICATIONLAUNCHER_H
#define BERRYMAINAPPLICATIONLAUNCHER_H

#include <ctkApplicationRunnable.h>

#include <QMutex>

namespace berry {

class ApplicationContainer;

/**
 * A main threaded application may be launched using this class to launch the main threaded application.
 */
class MainApplicationLauncher : public ctkApplicationRunnable
{
private:

  ApplicationContainer* appContainer;
  ctkApplicationRunnable* launchMainApp; // a handle to a main threaded application

  mutable QMutex mutex;

public:

  MainApplicationLauncher(ApplicationContainer* appContainer);

  QVariant run(const QVariant& context) override;

  void stop() override;

  void Launch(ctkApplicationRunnable* app);

private:

  ctkApplicationRunnable* GetMainHandle() const;

};

}

#endif // BERRYMAINAPPLICATIONLAUNCHER_H
