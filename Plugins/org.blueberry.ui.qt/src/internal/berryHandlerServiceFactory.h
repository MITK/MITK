/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHANDLERSERVICEFACTORY_H
#define BERRYHANDLERSERVICEFACTORY_H

#include "berryIServiceFactory.h"

namespace berry {

class HandlerServiceFactory : public QObject, public IServiceFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IServiceFactory)

public:

  Object* Create(const QString& serviceInterface, IServiceLocator* parentLocator,
                 IServiceLocator* locator) const override;
};

}

#endif // BERRYHANDLERSERVICEFACTORY_H
