/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(ServiceLocatorCreator);

  SmartPointer<IServiceLocator> CreateServiceLocator(
      IServiceLocator* parent,
      const IServiceFactory* factory,
      WeakPointer<IDisposable> owner) override;
};

}

#endif /* BERRYSERVICELOCATORCREATOR_H_ */
