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

#ifndef CHERRYIADAPTABLE_H_
#define CHERRYIADAPTABLE_H_

#include "cherryRuntimeDll.h"

#include <typeinfo>
#include <Poco/Any.h>

namespace cherry {

/**
 * An interface for an adaptable object.
 * <p>
 * Adaptable objects can be dynamically extended to provide different
 * interfaces (or "adapters").  Adapters are created by adapter
 * factories, which are in turn managed by type by adapter managers.
 * </p>
 * For example,
 * <pre>
 *     IAdaptable a = [some adaptable];
 *     IFoo x = (IFoo)a.getAdapter(IFoo.class);
 *     if (x != null)
 *         [do IFoo things with x]
 * </pre>
 * <p>
 * This interface can be used without OSGi running.
 * </p><p>
 * Clients may implement this interface, or obtain a default implementation
 * of this interface by subclassing <code>PlatformObject</code>.
 * </p>
 * @see IAdapterFactory
 * @see IAdapterManager
 * @see PlatformObject
 */
class IAdaptable {

public:

  /**
   * Returns an object which is an instance of the given class
   * associated with this object. Returns <code>null</code> if
   * no such object can be found.
   *
   * @param adapter the adapter class to look up
   * @return a object castable to the given class, 
   *    or <code>null</code> if this object does not
   *    have an adapter for the given class
   */
  template<typename A>
  A* GetAdapter() const
  {
    return static_cast<A*>(m_Dispatcher(m_Functor, this, typeid(A)));
  }

  virtual ~IAdaptable() { }

protected:

  template<typename AdapterFunctor>
  IAdaptable(AdapterFunctor func) :
    m_Functor(func), m_Dispatcher(&Dispatcher<AdapterFunctor>::Dispatch)
  {

  }

  IAdaptable() : m_Functor(DefaultDispatcher()),
                 m_Dispatcher(&Dispatcher<DefaultDispatcher>::Dispatch)
  {

  }

  virtual void* GetAdapterImpl(const std::type_info& /*type*/) const
  {
    return 0;
  }

private:

  typedef void*(*DispatchFunc)(Poco::Any&,const IAdaptable* const, const std::type_info&);

  template<typename AdapterFunctor_>
  struct Dispatcher {
    static void* Dispatch(Poco::Any& functor, const IAdaptable* const adaptable, const std::type_info& type)
    {
      return (Poco::RefAnyCast<AdapterFunctor_>(functor))(adaptable, type);
      //return (* static_cast<AdapterFunctor_ *>(functor))(adaptable, type);
    }
  };

  struct DefaultDispatcher {
    void* operator()(const IAdaptable* const adaptable, const std::type_info& type)
    {
      return adaptable->GetAdapterImpl(type);
    }
  };

  mutable Poco::Any m_Functor;
  DispatchFunc m_Dispatcher;

};

}  // namespace cherry

#endif /*CHERRYIADAPTABLE_H_*/
