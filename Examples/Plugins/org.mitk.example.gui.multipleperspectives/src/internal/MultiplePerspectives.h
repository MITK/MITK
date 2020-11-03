/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MULTIPLEPERSPECTIVES_H_
#define MULTIPLEPERSPECTIVES_H_

// berry includes
#include <berryIApplication.h>

// Qt includes
#include <QObject>
#include <QScopedPointer>

class MultiplePerspectivesWorkbenchAdvisor;

class MultiplePerspectives : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  MultiplePerspectives();
  ~MultiplePerspectives() override;

  QVariant Start(berry::IApplicationContext *context) override;
  void Stop() override;

private:
  QScopedPointer<MultiplePerspectivesWorkbenchAdvisor> wbAdvisor;
};

#endif /*MULTIPLEPERSPECTIVES_H_*/
