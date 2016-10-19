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

#ifndef QMITKQMLAPPLICATION_H
#define QMITKQMLAPPLICATION_H

#include <berryIApplication.h>
#include <berryWorkbenchAdvisor.h>

class QmitkQmlApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkQmlApplication();
  QmitkQmlApplication(const QmitkQmlApplication& other);

  QVariant Start(berry::IApplicationContext* context);
  void Stop();
};

#endif
