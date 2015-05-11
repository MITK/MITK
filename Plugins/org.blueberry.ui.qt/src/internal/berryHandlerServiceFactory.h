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
