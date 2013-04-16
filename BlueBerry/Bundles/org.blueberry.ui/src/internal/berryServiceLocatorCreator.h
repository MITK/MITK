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


#ifndef BERRYSERVICELOCATORCREATOR_H_
#define BERRYSERVICELOCATORCREATOR_H_

#include "berryIServiceLocatorCreator.h"

namespace berry {

/**
 * A simple service locator creator.
 *
 * @since 3.4
 */
class ServiceLocatorCreator : public IServiceLocatorCreator
{

public:

  berryObjectMacro(ServiceLocatorCreator)

  SmartPointer<IServiceLocator> CreateServiceLocator(
      const WeakPointer<IServiceLocator> parent,
      const SmartPointer<const IServiceFactory> factory,
      WeakPointer<IDisposable> owner);
};

}

#endif /* BERRYSERVICELOCATORCREATOR_H_ */
