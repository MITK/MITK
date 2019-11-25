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

#ifndef QMITKFLOWAPPLICATIONLICATION_H_
#define QMITKFLOWAPPLICATIONLICATION_H_

#include <berryIApplication.h>


class QmitkFlowApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkFlowApplication();

  QVariant Start(berry::IApplicationContext* context) override;
  void Stop() override;
};

#endif /*QMITKFLOWAPPLICATIONLICATION_H_*/
