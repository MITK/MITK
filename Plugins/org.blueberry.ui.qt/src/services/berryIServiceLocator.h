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


#ifndef BERRYISERVICELOCATOR_H_
#define BERRYISERVICELOCATOR_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryObject.h>
#include <berryMacros.h>
#include <berryLog.h>

#include <QObject>

#include <typeinfo>

namespace berry {

/**
 * <p>
 * A component with which one or more services are registered. The services can
 * be retrieved from this locator using the service type. For example:
 * </p>
 *
 * <pre>
 * IHandlerService* service = workbenchWindow->GetService<IHandlerService>();
 * </pre>
 *
 * <p>
 * This interface is not to be implemented or extended by clients.
 * </p>
 */
struct BERRY_UI_QT IServiceLocator : public virtual Object
{

  berryObjectMacro(berry::IServiceLocator, Object)

  ~IServiceLocator();

  /**
   * Retrieves the service corresponding to the given API.
   *
   * @tparam S
   *            This is the interface that the service implements. Must not be
   *            <code>null</code>.
   * @return The service, or <code>null</code> if no such service could be
   *         found.
   */
  template<class S>
  S* GetService()
  {
    const char* typeName = qobject_interface_iid<S*>();
    if (typeName == NULL)
    {
      BERRY_WARN << "Error getting service: Cannot get the interface id for type '" << Reflection::GetClassName<S>()
                 << "'. It is probably missing a Q_DECLARE_INTERFACE macro in its header.";
      return NULL;
    }
    Object* obj = this->GetService(typeName);
    S* service = dynamic_cast<S*>(obj);
    if (obj != NULL && service == NULL)
    {
      BERRY_WARN << "Error getting service: Class '" << obj->GetClassName() << "' cannot be cast to service interface "
                 << "'" << Reflection::GetClassName<S>() << "'";
    }
    return service;
  }

  /**
   * Whether this service exists within the scope of this service locator.
   * This does not include looking for the service within the scope of the
   * parents. This method can be used to determine whether a particular
   * service supports nesting in this scope.
   *
   * @tparam S
   *            This is the interface that the service implements. Must not be
   *            <code>null</code>.
   * @return <code>true</code> iff the service locator can find a service
   *         for the given interface; <code>false</code> otherwise.
   */
  template<class S>
  bool HasService() const
  {
    return this->HasService(qobject_interface_iid<S*>());
  }

  virtual Object* GetService(const QString& api) = 0;
  virtual bool HasService(const QString& api) const = 0;
};

}

#endif /* BERRYISERVICELOCATOR_H_ */
