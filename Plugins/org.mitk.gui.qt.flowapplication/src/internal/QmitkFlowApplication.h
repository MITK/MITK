/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
