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

#ifndef QMITKCOREAPPLICATION_H_
#define QMITKCOREAPPLICATION_H_

#include <berryIApplication.h>

#include <QObject>


class QmitkCoreApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkCoreApplication();

  int Start();
  void Stop();
};

#endif /*QMITKCOREAPPLICATION_H_*/
