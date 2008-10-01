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

#ifndef CHERRYIADAPTERMANAGER_H_
#define CHERRYIADAPTERMANAGER_H_

#include <cherryMacros.h>

#include "cherryRuntimeDll.h"
#include "cherryExpressionVariables.h"
#include "cherryIAdapterFactory.h"

#include <typeinfo>

namespace cherry {

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
 * <code>org.opencherry.core.runtime.adapters</code> extension point.  Factories registered
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
struct IAdapterManager : public Object {

  cherryClassMacro(IAdapterManager)
  
  virtual ~IAdapterManager() {}
  
  /**
   * This value can be returned to indicate that no applicable adapter factory 
   * was found. 
   * @since org.opencherry.equinox.common 3.3
   */
  static CHERRY_RUNTIME const int NONE;

  /**
   * This value can be returned to indicate that an adapter factory was found, 
   * but has not been loaded.
   * @since org.opencherry.equinox.common 3.3
   */
  static CHERRY_RUNTIME const int NOT_LOADED;

  /**
   * This value can be returned to indicate that an adapter factory is loaded.
   * @since org.opencherry.equinox.common 3.3
   */
  static CHERRY_RUNTIME const int LOADED;

  /**
   * Returns the types that can be obtained by converting <code>adaptableClass</code> 
   * via this manager. Converting means that subsequent calls to <code>getAdapter()</code>
   * or <code>loadAdapter()</code> could result in an adapted object.
   * <p>
   * Note that the returned types do not guarantee that
   * a subsequent call to <code>getAdapter</code> with the same type as an argument
   * will return a non-null result. If the factory's plug-in has not yet been
   * loaded, or if the factory itself returns <code>null</code>, then
   * <code>getAdapter</code> will still return <code>null</code>.
   * </p>
   * @param adaptableClass the adaptable class being queried  
   * @return an array of type names that can be obtained by converting 
   * <code>adaptableClass</code> via this manager. An empty array 
   * is returned if there are none.
   * @since 3.1
   */
  virtual void ComputeAdapterTypes(std::vector<const std::type_info&>& types, const std::type_info& adaptableClass) = 0;

  /**
   * Returns the class search order for a given class. The search order from a 
   * class with the definition <br>
   * <code>class X extends Y implements A, B</code><br>
   * is as follows:
   * <ul>
   * <li>the target's class: X
   * <li>X's superclasses in order to <code>Object</code>
   * <li>a breadth-first traversal of the target class's interfaces in the
   * order returned by <code>getInterfaces</code> (in the example, A and its
   * superinterfaces then B and its superinterfaces) </li>
   * </ul>
   * 
   * @param clazz the class for which to return the class order. 
   * @return the class search order for the given class. The returned
   * search order will minimally  contain the target class.
   * @since 3.1
   */
  //public Class[] computeClassOrder(Class clazz);

  /**
   * Returns an object which is an instance of the given class associated
   * with the given object. Returns <code>null</code> if no such object can
   * be found.
   * <p>
   * Note that this method will never cause plug-ins to be loaded. If the
   * only suitable factory is not yet loaded, this method will return <code>null</code>.
   * 
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterType the type of adapter to look up
   * @return an object castable to the given adapter type, or <code>null</code>
   * if the given adaptable object does not have an available adapter of the
   * given type
   */
  virtual ExpressionVariable::Pointer GetAdapter(ExpressionVariable::Pointer adaptable, const std::type_info& adapterType) = 0;

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
   * @since 3.0
   */
  virtual ExpressionVariable::Pointer GetAdapter(ExpressionVariable::Pointer adaptable, const std::string& adapterTypeName) = 0;

  /**
   * Returns whether there is an adapter factory registered that may be able
   * to convert <code>adaptable</code> to an object of type <code>adapterTypeName</code>.
   * <p>
   * Note that a return value of <code>true</code> does not guarantee that
   * a subsequent call to <code>getAdapter</code> with the same arguments
   * will return a non-null result. If the factory's plug-in has not yet been
   * loaded, or if the factory itself returns <code>null</code>, then
   * <code>getAdapter</code> will still return <code>null</code>.
   * 
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified class name of an adapter to
   * look up
   * @return <code>true</code> if there is an adapter factory that claims
   * it can convert <code>adaptable</code> to an object of type <code>adapterType</code>,
   * and <code>false</code> otherwise.
   * @since 3.0
   */
  virtual bool HasAdapter(ExpressionVariable::Pointer adaptable, const std::string& adapterType) = 0;

  /**
   * Returns a status of an adapter factory registered that may be able
   * to convert <code>adaptable</code> to an object of type <code>adapterTypeName</code>.
   * <p>
   * One of the following values can be returned:<ul>
   * <li>{@link org.opencherry.core.runtime.IAdapterManager#NONE} if no applicable adapter factory was found;</li>
   * <li>{@link org.opencherry.core.runtime.IAdapterManager#NOT_LOADED} if an adapter factory was found, but has not been loaded;</li>
   * <li>{@link org.opencherry.core.runtime.IAdapterManager#LOADED} if an adapter factory was found, and it is loaded.</li>
   * </ul></p>
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified class name of an adapter to
   * look up
   * @return a status of the adapter 
   * @since org.opencherry.equinox.common 3.3
   */
  virtual int QueryAdapter(ExpressionVariable::Pointer adaptable, const std::string& adapterType) = 0;

  /**
   * Returns an object that is an instance of the given class name associated
   * with the given object. Returns <code>null</code> if no such object can
   * be found.
   * <p>
   * Note that unlike the <code>getAdapter</code> methods, this method
   * will cause the plug-in that contributes the adapter factory to be loaded
   * if necessary. As such, this method should be used judiciously, in order
   * to avoid unnecessary plug-in activations. Most clients should avoid
   * activation by using <code>getAdapter</code> instead.
   * 
   * @param adaptable the adaptable object being queried (usually an instance
   * of <code>IAdaptable</code>)
   * @param adapterTypeName the fully qualified name of the type of adapter to look up
   * @return an object castable to the given adapter type, or <code>null</code>
   * if the given adaptable object does not have an available adapter of the
   * given type
   * @since 3.0
   */
  virtual ExpressionVariable::Pointer LoadAdapter(ExpressionVariable::Pointer adaptable, const std::type_info& adapterType) = 0;

  /**
   * Registers the given adapter factory as extending objects of the given
   * type.
   * <p>
   * If the type being extended is a class, the given factory's adapters are
   * available on instances of that class and any of its subclasses. If it is
   * an interface, the adapters are available to all classes that directly or
   * indirectly implement that interface.
   * </p>
   * 
   * @param factory the adapter factory
   * @param adaptable the type being extended
   * @see #unregisterAdapters(IAdapterFactory)
   * @see #unregisterAdapters(IAdapterFactory, Class)
   */
  virtual void RegisterAdapters(IAdapterFactory* factory, const std::type_info& adaptable) = 0;

  /**
   * Removes the given adapter factory completely from the list of registered
   * factories. Equivalent to calling <code>unregisterAdapters(IAdapterFactory,Class)</code>
   * on all classes against which it had been explicitly registered. Does
   * nothing if the given factory is not currently registered.
   * 
   * @param factory the adapter factory to remove
   * @see #registerAdapters(IAdapterFactory, Class)
   */
  virtual void UnregisterAdapters(IAdapterFactory* factory) = 0;

  /**
   * Removes the given adapter factory from the list of factories registered
   * as extending the given class. Does nothing if the given factory and type
   * combination is not registered.
   * 
   * @param factory the adapter factory to remove
   * @param adaptable one of the types against which the given factory is
   * registered
   * @see #registerAdapters(IAdapterFactory, Class)
   */
  virtual void UnregisterAdapters(IAdapterFactory* factory, const std::type_info& adaptable) = 0;
};

} // namespace cherry

#endif /*CHERRYIADAPTERMANAGER_H_*/
