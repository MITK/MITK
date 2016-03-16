/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
