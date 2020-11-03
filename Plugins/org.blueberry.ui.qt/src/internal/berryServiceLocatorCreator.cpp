/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryServiceLocatorCreator.h"

#include "berryServiceLocator.h"
#include "services/berryIServiceFactory.h"
#include "services/berryIDisposable.h"

namespace berry {

SmartPointer<IServiceLocator>
ServiceLocatorCreator::CreateServiceLocator(
      IServiceLocator* parent,
      const IServiceFactory* factory,
      WeakPointer<IDisposable> owner)
{
  IServiceLocator::Pointer locator(new ServiceLocator(parent, factory, owner));
  return locator;
}

}
