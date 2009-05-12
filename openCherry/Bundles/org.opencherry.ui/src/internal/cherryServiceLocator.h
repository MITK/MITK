/*=========================================================================

 Program:   openCherry Platform
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

#ifndef CHERRYSERVICELOCATOR_H_
#define CHERRYSERVICELOCATOR_H_

#include "../services/cherryIServiceLocator.h"
#include "../services/cherryINestable.h"
#include "../services/cherryIDisposable.h"

#include <map>
#include <vector>
#include <string>
#include <typeinfo>

namespace cherry
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

    const IServiceLocator::WeakPtr locator;
    const std::string& key;

  public:

    ParentLocator(const IServiceLocator::WeakPtr parent,
        const std::string& serviceInterface);

    /*
     * (non-Javadoc)
     *
     * @see org.opencherry.ui.services.IServiceLocator#getService(java.lang.Class)
     */
    Object::Pointer GetService(const std::string& api);

    /*
     * (non-Javadoc)
     *
     * @see org.opencherry.ui.services.IServiceLocator#hasService(java.lang.Class)
     */
    bool HasService(const std::string& api) const;
  };

  const SmartPointer<const IServiceFactory> factory;

  /**
   * The parent for this service locator. If a service can't be found in this
   * locator, then the parent is asked. This value may be <code>null</code>
   * if there is no parent.
   */
  IServiceLocator::WeakPtr parent;

  /**
   * The map of services This value is <code>null</code> until a service is
   * registered.
   */
  typedef std::map<const std::string, Object::Pointer> KeyToServiceMapType;
  mutable KeyToServiceMapType services;

  bool disposed;

  IDisposable::WeakPtr owner;

public:

  cherryObjectMacro(ServiceLocator)

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
  ServiceLocator(const IServiceLocator::WeakPtr parent, const SmartPointer<const IServiceFactory> factory,
      IDisposable::WeakPtr owner);

  void Activate();

  void Deactivate();

  void Dispose();

  Object::Pointer GetService(const std::string& key);

  bool HasService(const std::string& key) const;

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
  void RegisterService(const std::string& api, Object::Pointer service) const;

  /**
   * @return
   */
  bool IsDisposed() const;

  /**
   * Some services that were contributed to this locator are no longer available
   * (because the plug-in containing the AbstractServiceFactory is no longer
   * available). Notify the owner of the locator about this.
   */
  void UnregisterServices(const std::vector<std::string>& serviceNames);

};

}

#endif // CHERRYSERVICELOCATOR_H_
