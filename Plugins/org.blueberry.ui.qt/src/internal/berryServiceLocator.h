/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSERVICELOCATOR_H_
#define BERRYSERVICELOCATOR_H_

#include "services/berryIServiceLocator.h"
#include "services/berryINestable.h"
#include "services/berryIDisposable.h"


namespace berry
{

struct IServiceFactory;

class ServiceLocator: public IDisposable,
    public INestable,
    public IServiceLocator
{

private:

  bool activated;

  class ParentLocator: public IServiceLocator
  {

    IServiceLocator* const locator;
    const QString& key;

  public:

    ParentLocator(IServiceLocator* parent,
                  const QString& serviceInterface);

    /*
     * (non-Javadoc)
     *
     * @see org.blueberry.ui.services.IServiceLocator#getService(java.lang.Class)
     */
    Object* GetService(const QString& api) override;

    /*
     * (non-Javadoc)
     *
     * @see org.blueberry.ui.services.IServiceLocator#hasService(java.lang.Class)
     */
    bool HasService(const QString& api) const override;
  };

  const IServiceFactory* const factory;

  /**
   * The parent for this service locator. If a service can't be found in this
   * locator, then the parent is asked. This value may be <code>null</code>
   * if there is no parent.
   */
  IServiceLocator* parent;

  /**
   * The map of services This value is <code>null</code> until a service is
   * registered.
   */
  typedef QHash<const QString, Object*> KeyToServiceMapType;
  mutable KeyToServiceMapType services;

  bool disposed;

  IDisposable::WeakPtr owner;

  QList<Object::Pointer> managedFactoryServices;

public:

  berryObjectMacro(ServiceLocator);

  /**
   * Constructs a service locator with no parent.
   */
  ServiceLocator();

  /**
   * Constructs a service locator with the given parent.
   *
   * @param parent
   *            The parent for this service locator; this value may be
   *            <code>null</code>.
   * @param factory
   *            a local factory that can provide services at this level
   * @param owner
   */
  ServiceLocator(IServiceLocator* parent, const IServiceFactory* factory,
      IDisposable::WeakPtr owner);

  void Activate() override;

  void Deactivate() override;

  void Dispose() override;

  using IServiceLocator::GetService;
  Object* GetService(const QString& key) override;

  bool HasService(const QString& key) const override;

  /**
   * Registers a service with this locator. If there is an existing service
   * matching the same <code>api</code> and it implements
   * {@link IDisposable}, it will be disposed.
   *
   * @param api
   *            This is the interface that the service implements. Must not be
   *            <code>null</code>.
   * @param service
   *            The service to register. This must be some implementation of
   *            <code>api</code>. This value must not be <code>null</code>.
   */
  template<class S>
  void RegisterService(S* service) const
  {
    this->RegisterService(qobject_interface_iid<S*>(), service);
  }

  /**
   * @return
   */
  bool IsDisposed() const;

  /**
   * Some services that were contributed to this locator are no longer available
   * (because the plug-in containing the AbstractServiceFactory is no longer
   * available). Notify the owner of the locator about this.
   */
  void UnregisterServices(const QList<QString> &serviceNames);

private:

  void RegisterService(const QString& api, Object* service) const;

};

}

#endif // BERRYSERVICELOCATOR_H_
