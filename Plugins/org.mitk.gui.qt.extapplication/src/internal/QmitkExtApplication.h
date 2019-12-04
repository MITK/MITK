/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKEXTAPPLICATION_H_
#define QMITKEXTAPPLICATION_H_

#include <berryIApplication.h>


class QmitkExtApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkExtApplication();

  QVariant Start(berry::IApplicationContext* context) override;
  void Stop() override;
};

#endif /*QMITKEXTAPPLICATION_H_*/
