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
  ~ExtensionPointDefinition();

  int Start();
  void Stop();

private:

  QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor;
};

#endif /*EXTENSIONPOINTDEFINITION_H_*/
