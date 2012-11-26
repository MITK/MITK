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
  ~MinimalApplication();

  int Start();
  void Stop();

private:

  QScopedPointer<MinimalWorkbenchAdvisor> wbAdvisor;
};

#endif /*MINIMALAPPLICATION_H_*/
