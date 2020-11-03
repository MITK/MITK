/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef EXTENSIONPOINTDEFINITION_H_
#define EXTENSIONPOINTDEFINITION_H_

// berry includes
#include <berryIApplication.h>

// Qt includes
#include <QObject>
#include <QScopedPointer>

class MinimalWorkbenchAdvisor;

class ExtensionPointDefinition : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:
  ExtensionPointDefinition();
  ~ExtensionPointDefinition() override;

  QVariant Start(berry::IApplicationContext *context) override;
  void Stop() override;

private:
  QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor;
};

#endif /*EXTENSIONPOINTDEFINITION_H_*/
