/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISERVICEFACTORY_H_
#define BERRYISERVICEFACTORY_H_

#include <org_blueberry_ui_qt_Export.h>

#include <QObject>

namespace berry {

struct IServiceLocator;

class Object;

/**
 * A factory for creating services for use with the
 * <code>org.blueberry.ui.services</code> extension point. You are given a
 * service locator to look up other services, and can retrieve your parent
 * service (if one has already been created).
 */
struct BERRY_UI_QT IServiceFactory {

  virtual ~IServiceFactory();

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
  template<class S>
  S* Create(IServiceLocator* parentLocator, IServiceLocator* locator) const
  {
    return dynamic_cast<S*>(this->Create(qobject_interface_iid<S*>(), parentLocator, locator));
  }

  virtual Object* Create(const QString& serviceInterface,
                         IServiceLocator* parentLocator, IServiceLocator* locator) const = 0;
};

}

Q_DECLARE_INTERFACE(berry::IServiceFactory, "org.blueberry.ui.IServiceFactory")

#endif /* BERRYISERVICEFACTORY_H_ */
