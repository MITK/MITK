/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYISERVICEFACTORY_H_
#define BERRYISERVICEFACTORY_H_

#include "../berryUiDll.h"

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

namespace berry {

struct IServiceLocator;

/**
 * A factory for creating services for use with the
 * <code>org.blueberry.ui.services</code> extension point. You are given a
 * service locator to look up other services, and can retrieve your parent
 * service (if one has already been created).
 *
 * @since 3.4
 */
struct BERRY_UI IServiceFactory : public virtual Object {

  osgiInterfaceMacro(berry::IServiceFactory);

  /**
   * When a service locator cannot find a service it will request one from the
   * registry, which will call this factory create method.
   * <p>
   * You can use the locator to get any needed services and a parent service
   * locator will be provided if you need access to the parent service. If the
   * parent object return from the parent locator is not <code>null</code>
   * it can be cast to the service interface that is requested. The parent
   * service locator will only return the serviceInterface service.
   * </p>
   *
   * @param serviceInterface
   *            the service we need to create. Will not be <code>null</code>.
   * @param parentLocator
   *            A locator that can return a parent service instance if
   *            desired. The parent service can be cast to serviceInterface.
   *            Will not be <code>null</code>.
   * @param locator
   *            the service locator which can be used to retrieve dependent
   *            services. Will not be <code>null</code>
   * @return the created service or <code>null</code>
   */
  virtual Object::Pointer Create(const std::string& serviceInterface,
      const SmartPointer<const IServiceLocator> parentLocator, const SmartPointer<const IServiceLocator> locator) const = 0;
};

}

#endif /* BERRYISERVICEFACTORY_H_ */
