/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MINIMALAPPLICATION_H_
#define MINIMALAPPLICATION_H_

// Berry
#include <berryIApplication.h>

// Qt
#include <QObject>
#include <QScopedPointer>

class MinimalWorkbenchAdvisor;

class MinimalApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  MinimalApplication();
  ~MinimalApplication() override;

  QVariant Start(berry::IApplicationContext *context) override;
  void Stop() override;
};

#endif /*MINIMALAPPLICATION_H_*/
