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

#ifndef QMITKEXTAPPLICATION_H_
#define QMITKEXTAPPLICATION_H_

#include <berryIApplication.h>


class QmitkExtApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkExtApplication();

  int Start();
  void Stop();
};

#endif /*QMITKEXTAPPLICATION_H_*/
