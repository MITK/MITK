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

#ifndef BERRYIADAPTERMANAGER_H_
#define BERRYIADAPTERMANAGER_H_

#include <berryMacros.h>

#include <org_blueberry_core_runtime_Export.h>

#include "berryPlatformObject.h"
#include "berryIAdapterFactory.h"

#include <QtPlugin>

#include <typeinfo>

namespace berry
{

/**
 * An adapter manager maintains a registry of adapter factories. Clients
 * directly invoke methods on an adapter manager to register and unregister
 * adapters. All adaptable objects (that is, objects that implement the <code>IAdaptable</code>
 * interface) tunnel <code>IAdaptable.getAdapter</code> invocations to their
 * adapter manager's <code>IAdapterManger.getAdapter</code> method. The
 * adapter manager then forwards this request unmodified to the <code>IAdapterFactory.getAdapter</code>
 * method on one of the registered adapter factories.
 * <p>
 * Adapter factories can be registered programmatically using the <code>registerAdapters</code>
 * method.  Alternatively, they can be registered declaratively using the
 * <code>org.blueberry.core.runtime.adapters</code> extension point.  Factories registered
 * with this extension point will not be able to provide adapters until their
 * corresponding plugin has been activated.
 * <p>
 * The following code snippet shows how one might register an adapter of type
 * <code>com.example.acme.Sticky</code> on resources in the workspace.
 * <p>
 *
 * <pre>
 *  IAdapterFactory pr = new IAdapterFactory() {
 *    public Class[] getAdapterList() {
 *      return new Class[] { com.example.acme.Sticky.class };
 *    }
 *    public Object getAdapter(Object adaptableObject, Class adapterType) {
 *      IResource res = (IResource) adaptableObject;
 *      QualifiedName key = new QualifiedName(&quot;com.example.acme&quot;, &quot;sticky-note&quot;);
 *      try {
 *        com.example.acme.Sticky v = (com.example.acme.Sticky) res.getSessionProperty(key);
 *        if (v == null) {
 *          v = new com.example.acme.Sticky();
 *          res.setSessionProperty(key, v);
 *        }
 *      } catch (CoreException e) {
 *        // unable to access session property - ignore
 *      }
 *      return v;
 *    }
 *  }
 *  Platform.getAdapterManager().registerAdapters(pr, IResource.class);
 *   </pre>
 *
 * </p><p>
 * This interface can be used without OSGi running.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @see IAdaptable
 * @see IAdapterFactory
 */
struct org_blueberry_core_runtime_EXPORT IAdapterManager: public Object
{

  berryObjectMacro(berry::IAdapterManager)

  /**
   * This value can be returned to indicate that no applicable adapter factory
   * was found.
   * @since org.blueberry.equinox.common 3.3
   */
  static const int NONE;

  /**
   * This value can be returned to indicate that an adapter factory was found,
   * but has not been loaded.
   * @since org.blueberry.equinox.common 3.3
   */
  static const int NOT_LOADED;

  /**
   * This value can be returned to indicate that an adapter factory is loaded.
   * @since org.blueberry.equinox.common 3.3
   */
  static const int LOADED;

  /**
   * Returns a Poco::Any object which contains an instance of the given name associated
   * with the given adaptable. Returns an empty Poco::Any if no such object can
   * be found.
   * <p>
   * Note that this method will never cause plug-ins to be loaded. If the
   * only suitable factory is not yet loaded, this method will return an empty Poco::Any.
   * If activation of the plug-in providing the factory is required, use the
   * <code>LoadAdapter</code> method instead.
   *
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified name of the type of adapter to look up
   * @return a Poco::Any castable to the given adapter type, or empty
   * if the given adaptable object does not have an available adapter of the
   * given type
   */
  template<typename A>
  A* GetAdapter(const Object* adaptable)
  {
    const char* typeName = qobject_interface_iid<A*>();
    if (typeName == nullptr)
    {
      BERRY_WARN << "Error getting adapter for '" << Reflection::GetClassName(adaptable) << "': "
                 << "Cannot get the interface id for type '" << Reflection::GetClassName<A>()
                 << "'. It is probably missing a Q_DECLARE_INTERFACE macro in its header.";
      return nullptr;
    }
    return dynamic_cast<A*>(this->GetAdapter(adaptable, typeName, false));
  }

  /**
   * Returns an object which is an instance of the given class name associated
   * with the given object. Returns <code>null</code> if no such object can
   * be found.
   * <p>
   * Note that this method will never cause plug-ins to be loaded. If the
   * only suitable factory is not yet loaded, this method will return <code>null</code>.
   * If activation of the plug-in providing the factory is required, use the
   * <code>loadAdapter</code> method instead.
   *
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified name of the type of adapter to look up
   * @return an object castable to the given adapter type, or <code>null</code>
   * if the given adaptable object does not have an available adapter of the
   * given type
   */
  virtual Object* GetAdapter(const Object* adaptable, const QString& adapterTypeName) = 0;

  /**
   * Returns whether there is an adapter factory registered that may be able
   * to convert <code>adaptable</code> to an object of type <code>adapterTypeName</code>.
   * <p>
   * Note that a return value of <code>true</code> does not guarantee that
   * a subsequent call to <code>GetAdapter</code> with the same arguments
   * will return a non-empty result. If the factory's plug-in has not yet been
   * loaded, or if the factory itself returns nothing, then
   * <code>GetAdapter</code> will still return an empty Poco::Any.
   *
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified class name of an adapter to
   * look up
   * @return <code>true</code> if there is an adapter factory that claims
   * it can convert <code>adaptable</code> to an object of type <code>adapterType</code>,
   * and <code>false</code> otherwise.
   */
  virtual bool HasAdapter(const Object* adaptableType, const QString& adapterType) = 0;

  template<typename A>
  int QueryAdapter(const Object* adaptable)
  {
    const char* typeName = qobject_interface_iid<A*>();
    if (typeName == nullptr)
    {
      BERRY_WARN << "Error querying adapter manager for '" << Reflection::GetClassName(adaptable) << "': "
                 << "Cannot get the interface id for type '" << Reflection::GetClassName<A>()
                 << "'. It is probably missing a Q_DECLARE_INTERFACE macro in its header.";
      return NONE;
    }
    return this->QueryAdapter(adaptable, typeName);
  }

  /**
   * Returns a status of an adapter factory registered that may be able
   * to convert <code>adaptable</code> to an object of type <code>adapterTypeName</code>.
   * <p>
   * One of the following values can be returned:<ul>
   * <li>{@link berry::IAdapterManager::NONE} if no applicable adapter factory was found;</li>
   * <li>{@link berry::IAdapterManager::NOT_LOADED} if an adapter factory was found, but has not been loaded;</li>
   * <li>{@link berry::IAdapterManager::LOADED} if an adapter factory was found, and it is loaded.</li>
   * </ul></p>
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified class name of an adapter to
   * look up
   * @return a status of the adapter
   */
  virtual int QueryAdapter(const Object* adaptableType, const QString& adapterType) = 0;

  /**
   * Returns an object that is an instance of the given class name associated
   * with the given object. Returns an empty Poco::Any if no such object can
   * be found.
   * <p>
   * Note that unlike the <code>GetAdapter</code> methods, this method
   * will cause the plug-in that contributes the adapter factory to be loaded
   * if necessary. As such, this method should be used judiciously, in order
   * to avoid unnecessary plug-in activations. Most clients should avoid
   * activation by using <code>GetAdapter</code> instead.
   *
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified name of the type of adapter to look up
   * @return a Poco::Any castable to the given adapter type, or empty
   * if the given adaptable object does not have an available adapter of the
   * given type
   */
  template<typename A>
  A* LoadAdapter(const Object* adaptable)
  {
    const char* typeName = qobject_interface_iid<A*>();
    if (typeName == nullptr)
    {
      BERRY_WARN << "Error getting adapter for '" << Reflection::GetClassName(adaptable) << "': "
                 << "Cannot get the interface id for type '" << Reflection::GetClassName<A>()
                 << "'. It is probably missing a Q_DECLARE_INTERFACE macro in its header.";
      return nullptr;
    }
    return dynamic_cast<A*>(this->GetAdapter(adaptable, typeName, true));
  }

  /**
   * Registers the given adapter factory as extending objects of the given
   * type.
   *
   * @param factory the adapter factory
   * @param adaptableTypeName the fully qualified typename being extended
   * @see #UnregisterAdapters(IAdapterFactory*)
   * @see #UnregisterAdapters(IAdapterFactory*, const std::adaptableTypeName&)
   */
  virtual void RegisterAdapters(IAdapterFactory* factory,
                                const QString& adaptableTypeName) = 0;

  /**
   * Removes the given adapter factory completely from the list of registered
   * factories. Equivalent to calling <code>UnregisterAdapters(IAdapterFactory*, const std::string&)</code>
   * on all classes against which it had been explicitly registered. Does
   * nothing if the given factory is not currently registered.
   *
   * @param factory the adapter factory to remove
   * @see #RegisterAdapters(IAdapterFactory*, const std::string&)
   */
  virtual void UnregisterAdapters(IAdapterFactory* factory) = 0;

  /**
   * Removes the given adapter factory from the list of factories registered
   * as extending the given class. Does nothing if the given factory and type
   * combination is not registered.
   *
   * @param factory the adapter factory to remove
   * @param adaptableTypeName one of the type names against which the given factory is
   * registered
   * @see #RegisterAdapters(IAdapterFactory*, const std::string&)
   */
  virtual void UnregisterAdapters(IAdapterFactory* factory,
                                  const QString& adaptableTypeName) = 0;

private:

  virtual Object* GetAdapter(const Object* adaptable, const QString& adapterType, bool force) = 0;
};

} // namespace berry

Q_DECLARE_INTERFACE(berry::IAdapterManager, "org.blueberry.service.IAdapterManager")

#endif /*BERRYIADAPTERMANAGER_H_*/
