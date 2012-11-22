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
  ~MultiplePerspectives();

  int Start();
  void Stop();

private:

  QScopedPointer<MultiplePerspectivesWorkbenchAdvisor> wbAdvisor;
};

#endif /*MULTIPLEPERSPECTIVES_H_*/
