/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
