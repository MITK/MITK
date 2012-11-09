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

#include "berryServiceLocatorCreator.h"

#include "berryServiceLocator.h"
#include "services/berryIServiceFactory.h"
#include "services/berryIDisposable.h"

namespace berry {

SmartPointer<IServiceLocator>
ServiceLocatorCreator::CreateServiceLocator(
      const WeakPointer<IServiceLocator> parent,
      const SmartPointer<const IServiceFactory> factory,
      WeakPointer<IDisposable> owner)
{
  IServiceLocator::WeakPtr weakParent(parent);
  IServiceLocator::Pointer locator(new ServiceLocator(weakParent, factory, owner));
  return locator;
}

}
