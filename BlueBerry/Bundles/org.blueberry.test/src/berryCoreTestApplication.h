/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYCORETESTAPPLICATION_H_
#define BERRYCORETESTAPPLICATION_H_

#include <berryIApplication.h>

#include <QObject>

namespace berry {

class CoreTestApplication : public QObject, public IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  CoreTestApplication();
  CoreTestApplication(const CoreTestApplication& other);

  int Start();
  void Stop();
};

}

#endif /* BERRYCORETESTAPPLICATION_H_ */
