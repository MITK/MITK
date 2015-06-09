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
